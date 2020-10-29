/* ========================================================================
   $File: work/tools/blandwidth/win32_blandwidth.c $
   $Date: 2020/06/18 00:32:25 UTC $
   $Revision: 2 $
   $Creator: Casey Muratori $
   ======================================================================== */

#include <windows.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <wmmintrin.h>
#ifdef __clang__
#include <intrin.h>
#include <avxintrin.h>
#include <avx2intrin.h>
#include <avx512fintrin.h>
#endif

#include "blandwidth.h"

#define TIME_OPEN(Stamp) QueryPerformanceCounter((LARGE_INTEGER *)&(Stamp).Counter); (Stamp).Clock = __rdtsc()
#define TIME_CLOSE(Stamp) (Stamp).Clock = __rdtsc(); QueryPerformanceCounter((LARGE_INTEGER *)&(Stamp).Counter)

#include "x64_blandwidth.c"
#include "blandwidth.c"
    
typedef struct win32_queues
{
    HANDLE Result;
    HANDLE Dispatch;
} win32_queues;

typedef struct win32_context
{
    context Context;
    win32_queues Queues;
} win32_context;

function void
DispatchWork(context *Context, u32 OpCount, memory_operation *Ops)
{
    win32_context *Win32Context = (win32_context *)Context;
    for(u32 OpIndex = 0;
        OpIndex < OpCount;
        ++OpIndex)
    {
        PostQueuedCompletionStatus(Win32Context->Queues.Dispatch, 0, 0, (OVERLAPPED *)(Ops + OpIndex));
    }
}

function memory_operation *
ReceiveWorkResult(context *Context)
{
    win32_context *Win32Context = (win32_context *)Context;
    DWORD IgnoredBytes;
    ULONG_PTR IgnoredKey;
    OVERLAPPED *Overlapped;
    GetQueuedCompletionStatus(Win32Context->Queues.Result, &IgnoredBytes, &IgnoredKey, &Overlapped, INFINITE);
    memory_operation *Result = (memory_operation *)Overlapped;
    return(Result);
}

function void
Statusf(char const *Format, ...)
{
    char Buffer[1024];
    
    va_list Args;
    va_start(Args, Format);
    u32 Length = wvsprintf(Buffer, Format, Args);
    va_end(Args);

    DWORD Ignored;
    WriteFile(GetStdHandle(STD_ERROR_HANDLE), Buffer, Length, &Ignored, 0);
}

function void
Dataf(char const *Format, ...)
{
    char Buffer[1024];
    
    va_list Args;
    va_start(Args, Format);
    u32 Length = wvsprintf(Buffer, Format, Args);
    va_end(Args);

    DWORD Ignored;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), Buffer, Length, &Ignored, 0);
}

function void *
AllocateAndClear(u64 Size)
{
    void *Result = VirtualAlloc(0, Size, MEM_COMMIT, PAGE_READWRITE);
    if(!Result)
    {
        Statusf("ERROR: Unable to allocate required memory.\n");
        ExitProcess(1);
    }
    
    return(Result);
}

function DWORD WINAPI
ThreadEntryPoint(void *Passthrough)
{
    win32_queues *Queues = (win32_queues *)Passthrough;
    
    for (;;)
    {
        DWORD IgnoredBytes;
        ULONG_PTR IgnoredKey;
        OVERLAPPED *Overlapped;
        GetQueuedCompletionStatus(Queues->Dispatch, &IgnoredBytes, &IgnoredKey, &Overlapped, INFINITE);
        if (Overlapped == NULL) /* special termination packet */
                break;
        memory_operation *Op = (memory_operation *)Overlapped;
        
        do
        {
            TIME_OPEN(Op->StartStamp);
        } while(Op->StartStamp.Counter < Op->StartGateCounter);
        
        Op->Handler(Op);
        TIME_CLOSE(Op->EndStamp);
        
        PostQueuedCompletionStatus(Queues->Result, 0, 0, Overlapped);
    }
    
    return(0);
}

void __cdecl
mainCRTStartup(void)
{
    //
    // NOTE(casey): Determine base timer and CPU frequencies
    //
    
    time BaseHz;
    QueryPerformanceFrequency((LARGE_INTEGER *)&BaseHz.Counter);
    BaseHz.Clock = 0;
    
    time SleepBegin;
    TIME_OPEN(SleepBegin);
    Sleep(1000);
    time SleepEnd;
    TIME_CLOSE(SleepEnd);
    
    time Delta = Subtract(SleepEnd, SleepBegin);
    BaseHz.Clock = (Delta.Clock * BaseHz.Counter) / Delta.Counter;
    
    //
    // NOTE(casey): Configure testing setup based on CPU parameters
    //

    SYSTEM_INFO SysInfo = {0};
    GetSystemInfo(&SysInfo);
    u32 MaxThreadCount = SysInfo.dwNumberOfProcessors;
    
    int CID[4] = {0};
    char CPUBrand[64] = {0};
    __cpuidex((int *)(CPUBrand + 0), 0x80000002, 0);
    __cpuidex((int *)(CPUBrand + 16), 0x80000003, 0);
    __cpuidex((int *)(CPUBrand + 32), 0x80000004, 0);
    
    handler_table_entry MemoryHandlers[] =
    {
        HANDLER_ENTRY(X64Read128), HANDLER_ENTRY(X64Write128), HANDLER_ENTRY(X64ReadWrite128),
        HANDLER_ENTRY(X64Read256), HANDLER_ENTRY(X64Write256), HANDLER_ENTRY(X64ReadWrite256),
        HANDLER_ENTRY(X64Read512), HANDLER_ENTRY(X64Write512), HANDLER_ENTRY(X64ReadWrite512),
    };
    
    u32 HandlerCount = 3; // NOTE(casey): We assume SSE for all x64 chips
    
    __cpuidex(CID, 1, 0);
    if((CID[2] & (1 << 28)))
    {
        HandlerCount = 6; // NOTE(casey): AVX is supported
        __cpuidex(CID, 7, 0);
        if((CID[1] & (1 << 16)))
        {
            HandlerCount = 9; // NOTE(casey): AVX-512 is supported
        }
    }
    
    //
    // NOTE(casey): Prepare threads
    //
    
    win32_queues Queues;
    Queues.Dispatch = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    Queues.Result = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    
    DWORD *ThreadIDs = (DWORD *)AllocateAndClear(MaxThreadCount*SizeOf(DWORD));
    HANDLE *ThreadHandles = (HANDLE *)AllocateAndClear(MaxThreadCount*SizeOf(DWORD));
    for(u32 ThreadIndex = 0;
        ThreadIndex < MaxThreadCount;
        ++ThreadIndex)
    {
        u32 StackSize = 1024*1024; // NOTE(casey): We need extremely little stack space, but we don't know how much the OS might need, so we err high
        HANDLE Thread = CreateThread(0, StackSize, ThreadEntryPoint, &Queues, 0, ThreadIDs + ThreadIndex);
        if(Thread == INVALID_HANDLE_VALUE)
        {
            Statusf("Unable to create thread");
            ExitProcess(2);
        }        
        ThreadHandles[ThreadIndex] = Thread;
    }
    
    //
    // NOTE(casey): Run tests
    //
    
    win32_context Win32Context = {0};
    Win32Context.Context.BaseHz = BaseHz;
    Win32Context.Context.MaxThreadCount = MaxThreadCount;
    Win32Context.Context.HandlerCount = HandlerCount;
    Win32Context.Context.Handlers = MemoryHandlers;
    Win32Context.Context.CPUBrand = CPUBrand;
    Win32Context.Context.LogicalCoreCount = MaxThreadCount;
    Win32Context.Queues = Queues;
    
    Main(&Win32Context.Context);

    //
    // NOTE: This is a crude way to notify all worker threads that they should
    // terminate. We send #threads termination packets to the completion port.
    // Since each thread should terminate after reading one termination packet
    // this should terminate all threads.
    //
    // A problem with this that we accept is that this is not very resilient
    // to bugs. If, suppose, we don't know the exact number of workers on the
    // IOCP (it might be dynamic, or some workers might have prematurely
    // terminated, etc) then we might end up posting too few or too many
    // completion packets. Both would be a problem, since (I assume) sending
    // too many can block the sender.
    //
    // I currently do not know a better way to notify IOCP workers. We could
    // use a manual-reset event that all workers wait for, while simultaneously
    // waiting for packets to arrive at the the completion port. The problem
    // with this approach is that the IOCP would (I assume) signal all workers
    // when there is a single packet available. This is called the thundering
    // herd problem. So it seems that we're probably forced to route all
    // messages to workers through the IOCP. That is one reason why I'm not so
    // convinced that IOCP is a good design. But I don't know - maybe there is
    // an actually good way to handle termination.
    //

    for (u32 ThreadIndex = 0;
            ThreadIndex < MaxThreadCount;
            ++ThreadIndex)
    {
        // Send special termination packet (numBytes, key, OVERLAPPED pointer all set to NULL)
        PostQueuedCompletionStatus(Win32Context.Queues.Dispatch, 0, 0, NULL);
    }

    //
    // NOTE: wait for all threads to terminate
    //

    for (u32 ThreadIndex = 0;
            ThreadIndex < MaxThreadCount;
            ++ThreadIndex)
    {
            Statusf("Waiting for thread %d (thread id %d)", (int)ThreadIndex, (int)ThreadIDs[ThreadIndex]);
            if (WaitForSingleObject(ThreadHandles[ThreadIndex], INFINITE) != WAIT_OBJECT_0)
            {
                    Statusf("ERROR: Wait failed");
                    ExitProcess(1);
            }
    }
    
    //
    // NOTE(casey): Exit
    //
    
    ExitProcess(0);
}

#undef function
#pragma function(memset)
void *memset(void *DestInit, int SourceInit, size_t Size)
{
    unsigned char Source = *(unsigned char *)&SourceInit;
    unsigned char *Dest = (unsigned char *)DestInit;
    while(Size--)
    {
        *Dest++ = Source;
    }
    
    return(DestInit);
}

#pragma function(memcpy)
void *memcpy(void *DestInit, void const *SourceInit, size_t Size)
{
    unsigned char *Source = (unsigned char *)SourceInit;
    unsigned char *Dest = (unsigned char *)DestInit;
    while(Size--)
    {
        *Dest++ = *Source++;
    }
    return(DestInit);
}

