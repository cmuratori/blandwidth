#ifndef __AVX__
#define __AVX__
#endif
#ifndef __AVX2__
#define __AVX2__
#endif
#ifndef __AVX512F__
#define __AVX512F__
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cpuid.h>
#include <immintrin.h>
#include <sched.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>

#include "blandwidth.h"

#define TIME_OPEN(stamp)                                                  \
  ({                                                                      \
    struct timespec _a;                                                   \
    clock_gettime(CLOCK_MONOTONIC, &_a);                                  \
    (stamp).Counter = ((u64)_a.tv_sec * 1000000000ull) + (u64)_a.tv_nsec; \
    (stamp).Clock = __rdtsc();                                            \
  })

#define TIME_CLOSE(stamp)                                                 \
  ({                                                                      \
    (stamp).Clock = __rdtsc();                                            \
    struct timespec _a;                                                   \
    clock_gettime(CLOCK_MONOTONIC, &_a);                                  \
    (stamp).Counter = ((u64)_a.tv_sec * 1000000000ull) + (u64)_a.tv_nsec; \
  })

#include "blandwidth.c"
#include "x64_blandwidth.c"

typedef struct work_node {
  union {
    struct work_node* Next;
    struct work_node* NextFree;
  };
  memory_operation* Op;
} work_node;

typedef struct {
  sem_t DispatchSem;
  sem_t ResultSem;
  work_node* DispatchHead;
  work_node* ResultHead;
  work_node* FirstFree;
} linux_queues;

typedef struct {
  context Context;
  linux_queues Queues;
} linux_context;

// NOTE(btolsch): DispatchWork and ReceiveWorkResult are single-threaded so these don't need to be
// thread-safe.
#define FREE_LIST_ALLOC(Head) \
  Head;                       \
  Head = (Head)->NextFree;
#define FREE_LIST_FREE(Head, Node) \
  (Node)->NextFree = Head;         \
  Head = Node;

function void
EnqueueWork(work_node** Head, work_node* Node) {
  work_node* CurrentHead = *Head;
  work_node* TempHead = 0;
  Node->Next = CurrentHead;
  while ((TempHead = __sync_val_compare_and_swap(Head, CurrentHead, Node)) != CurrentHead) {
    CurrentHead = TempHead;
    Node->Next = CurrentHead;
  }
}

function work_node*
DequeueWork(work_node** Head) {
  work_node* CurrentHead = *Head;
  work_node* Result = 0;
  while (CurrentHead && !Result) {
    work_node* CurrentNext = CurrentHead->Next;
    work_node* TempHead = 0;
    if ((TempHead = __sync_val_compare_and_swap(Head, CurrentHead, CurrentNext)) == CurrentHead) {
      Result = CurrentHead;
    }
    CurrentHead = TempHead;
  }
  return Result;
}

function void
DispatchWork(context* Context, u32 OpCount, memory_operation* Ops) {
  linux_context* LinuxContext = (linux_context*)Context;
  for (u32 OpIndex = 0; OpIndex < OpCount; ++OpIndex) {
    work_node* Work = FREE_LIST_ALLOC(LinuxContext->Queues.FirstFree);
    Work->Op = Ops + OpIndex;
    EnqueueWork(&LinuxContext->Queues.DispatchHead, Work);
    sem_post(&LinuxContext->Queues.DispatchSem);
  }
}

function memory_operation*
ReceiveWorkResult(context* Context) {
  linux_context* LinuxContext = (linux_context*)Context;
  work_node* Work = 0;
  while (!Work) {
    sem_wait(&LinuxContext->Queues.ResultSem);
    Work = DequeueWork(&LinuxContext->Queues.ResultHead);
  }

  memory_operation* Result = Work->Op;
  FREE_LIST_FREE(LinuxContext->Queues.FirstFree, Work);

  return Result;
}

function u32
StringfConvert(char* Buffer, char const* Format, va_list Args) {
  char ConvertedFormat[2048];
  u32 NextSourceIndex = 0;
  u32 NextDestIndex = 0;
  u32 Index = 0;
  // NOTE(btolsch): On x64, %Iu converts to %lu and %lu/%u both convert to %u.
  while (Format[Index]) {
    if (Format[Index++] == '%') {
      b32 Searching = 1;
      while (*Format && Searching) {
        char c = Format[Index++];
        switch (c) {
          case '%':
          case 'c':
          case 'C':
          case 'd':
          case 's':
          case 'S':
          case 'u':
          case 'i':
          case 'x':
          case 'X':
          case 'p': Searching = 0; break;
          case 'l': {
            u32 CopySize = Index - NextSourceIndex - 1;
            memcpy(ConvertedFormat + NextDestIndex, Format + NextSourceIndex, CopySize);
            NextSourceIndex = Index;
            NextDestIndex += CopySize;
            Searching = 0;
          } break;
          case 'I': {
            u32 CopySize = Index - NextSourceIndex - 1;
            memcpy(ConvertedFormat + NextDestIndex, Format + NextSourceIndex, CopySize);
            NextDestIndex += CopySize;
            NextSourceIndex = Index;
            ConvertedFormat[NextDestIndex++] = 'l';
            Searching = 0;
          } break;
          default: break;
        }
      }
      if (Searching) {
        exit(1);
      }
    }
  }

  u32 CopySize = Index - NextSourceIndex;
  memcpy(ConvertedFormat + NextDestIndex, Format + NextSourceIndex, CopySize);
  ConvertedFormat[Index] = 0;

  u32 Length = vsprintf(Buffer, ConvertedFormat, Args);

  return Length;
}

function void
Statusf(char const* Format, ...) {
  char Buffer[2048];

  va_list Args;
  va_start(Args, Format);
  u32 Length = StringfConvert(Buffer, Format, Args);
  va_end(Args);

  write(STDERR_FILENO, Buffer, Length);
}

function void
Dataf(char const* Format, ...) {
  char Buffer[2048];

  va_list Args;
  va_start(Args, Format);
  u32 Length = StringfConvert(Buffer, Format, Args);
  va_end(Args);

  write(STDOUT_FILENO, Buffer, Length);
}

function u32
Stringf(char* Buffer, char const* Format, ...) {
  va_list Args;
  va_start(Args, Format);
  u32 Length = StringfConvert(Buffer, Format, Args);
  va_end(Args);
  return Length;
}

function void*
AllocateAndClear(u64 Size) {
  void* Result = mmap(0, Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (Result == MAP_FAILED) {
    Statusf("ERROR: Unable to allocate required memory.\n");
    exit(1);
  }
  return Result;
}

function int
ThreadEntryPoint(void* Arg) {
  linux_context* LinuxContext = (linux_context*)Arg;
  while (1) {
    sem_wait(&LinuxContext->Queues.DispatchSem);
    work_node* Work = DequeueWork(&LinuxContext->Queues.DispatchHead);
    if (Work) {
      memory_operation* Op = Work->Op;
      do {
        TIME_OPEN(Op->StartStamp);
      } while (Op->StartStamp.Counter < Op->StartGateCounter);

      Op->Handler(Op);
      TIME_CLOSE(Op->EndStamp);

      EnqueueWork(&LinuxContext->Queues.ResultHead, Work);
      sem_post(&LinuxContext->Queues.ResultSem);
    }
  }
  return 0;
}

int
main(int argc, char** argv) {
  // NOTE(btolsch): Initialize test setup based on CPU parameters.
  timestamp BaseHz = {.Counter = 1000000000ull};

  timestamp SleepBegin;
  TIME_OPEN(SleepBegin);
  usleep(1000000);
  timestamp SleepEnd;
  TIME_CLOSE(SleepEnd);

  timestamp Delta = Subtract(SleepEnd, SleepBegin);
  BaseHz.Clock = (Delta.Clock * BaseHz.Counter) / Delta.Counter;

  int CID[4] = {0};
  char CPUBrand[64] = {0};
  __cpuid(0x80000002, ((int*)CPUBrand)[0], ((int*)CPUBrand)[1], ((int*)CPUBrand)[2],
          ((int*)CPUBrand)[3]);
  __cpuid(0x80000003, ((int*)CPUBrand)[4], ((int*)CPUBrand)[5], ((int*)CPUBrand)[6],
          ((int*)CPUBrand)[7]);
  __cpuid(0x80000004, ((int*)CPUBrand)[8], ((int*)CPUBrand)[9], ((int*)CPUBrand)[10],
          ((int*)CPUBrand)[11]);

  u32 MaxThreadCount = get_nprocs();

  handler_table_entry MemoryHandlers[] = {
      HANDLER_ENTRY(X64Read128), HANDLER_ENTRY(X64Write128), HANDLER_ENTRY(X64ReadWrite128),
      HANDLER_ENTRY(X64Read256), HANDLER_ENTRY(X64Write256), HANDLER_ENTRY(X64ReadWrite256),
      HANDLER_ENTRY(X64Read512), HANDLER_ENTRY(X64Write512), HANDLER_ENTRY(X64ReadWrite512),
  };

  // NOTE(btolsch): Assume SSE for x64, then determine AVX and AVX512 support.
  u32 HandlerCount = 3;
  __cpuid(1, CID[0], CID[1], CID[2], CID[3]);
  if ((CID[2] & (1 << 28))) {
    HandlerCount = 6;
    __cpuid(7, CID[0], CID[1], CID[2], CID[3]);
    if ((CID[1] & (1 << 16))) {
      HandlerCount = 9;
    }
  }

  // NOTE(btolsch): Prepare threads.
  linux_context LinuxContext = {};
  LinuxContext.Context.BaseHz = BaseHz;
  LinuxContext.Context.MaxThreadCount = MaxThreadCount;
  LinuxContext.Context.HandlerCount = HandlerCount;
  LinuxContext.Context.Handlers = MemoryHandlers;
  LinuxContext.Context.CPUBrand = CPUBrand;
  LinuxContext.Context.LogicalCoreCount = MaxThreadCount;

  {
    sem_init(&LinuxContext.Queues.DispatchSem, 0, 0);
    sem_init(&LinuxContext.Queues.ResultSem, 0, 0);

    u32 TotalWorkCount = 4 * MaxThreadCount;
    u32 TotalWorkSize = sizeof(work_node) * TotalWorkCount;
    work_node* Nodes = (work_node*)AllocateAndClear(TotalWorkSize);
    for (u32 WorkIndex = 0; WorkIndex < TotalWorkCount; ++WorkIndex) {
      FREE_LIST_FREE(LinuxContext.Queues.FirstFree, Nodes + WorkIndex);
    }
  }

  u32 StackSize = 1024 * 1024;  // NOTE(btolsch): Mostly chosen for parity with win32 platform.
  u32 CloneFlags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SYSVSEM | CLONE_SIGHAND | CLONE_THREAD;
  for (u32 ThreadIndex = 0; ThreadIndex < MaxThreadCount; ++ThreadIndex) {
    void* Stack = AllocateAndClear(StackSize);
    int Result = clone(&ThreadEntryPoint, (u8*)Stack + StackSize, CloneFlags, &LinuxContext);
    if (Result < 0) {
      Statusf("Unable to start thread.\n");
      exit(1);
    }
  }

  // NOTE(btolsch): Run tests.
  Main(&LinuxContext.Context);

  exit(0);
}
