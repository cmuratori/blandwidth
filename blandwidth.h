/* ========================================================================
   $File: work/tools/blandwidth/blandwidth.h $
   $Date: 2020/06/17 05:03:59 UTC $
   $Revision: 4 $
   $Creator: Casey Muratori $
   ======================================================================== */

/* TODO(casey):

   Currently, there are few things missing that are important for gaining
   insight into how different x64 architectures supply bandwidth.  Future
   versions of Blandwidth should add:
   
   1) Logical core affinity and scattered test patterns to measure the
      difference between cores as well as the differences when a core
      has two hyperthreads vs. one.
      
   2) Reading out of small buffers but writing into large ones, and
      reading out of large buffers but writing into small ones, since
      these patterns may change the behavior.
      
   3) Randomized read/write offsets, to measure the bandwidth when the
      CPU cannot predict the next offset.
      
   4) Reading and writing the same buffer, instead of reading from
      one buffer and writing to another.
      
   In addition, there are some general quality problems that I expect could
   be improved:
   
   1) The current "starting gate" system for trying to make multiple threads
      start processing at roughly the same time is not particularly good.
      Perhaps RDTSC should be used instead of QueryPerformanceCounter, but
      really it probably doesn't actually help one way or the other, and
      the ideal thing would be an OS strobe of some kind.
      
   2) There hasn't been any real CPUID testing, so it may be erroneously
      enabling AVX when it should not.  I did not use the try/except formulation
      here because doing so requires implementing a stack handler, which
      I didn't think was worth it.
      
   3) It's unclear that timestamps should bother with RDTSC at all, since
      it is not currently used for statistics and it is not as stable with
      respect to core boosting as QueryPerformanceCounter should theoretically
      be.  Perhaps switch to just counters, no clocks, for the stats?
*/

#define VERSION_STRING "r1"
#define BLOCK_SIZE 128

#ifdef LLVM_MCA
#define MCA_BEGIN(Region) __asm volatile("# LLVM-MCA-BEGIN" #Region)
#define MCA_END(Region) __asm volatile("# LLVM-MCA-END" #Region)
#else
#define MCA_BEGIN(...)
#define MCA_END(...)
#endif

#ifdef __clang__
#define function_avx2 static __attribute__ ((__target__("avx2")))
#define function_avx512 static __attribute__ ((__target__("avx512f")))
#define CTAssert(TestExpression) // TODO(casey): How do I get a static assert in C in CLANG?
#else
#define function_avx2 static
#define function_avx512 static
#define CTAssert(TestExpression) static_assert(TestExpression, "Expression not true: (" #TestExpression ")")
#endif

#define function static
#define global static

#define SizeOf(thing) sizeof(thing)
#define ArrayCount(Array) (SizeOf(Array) / SizeOf((Array)[0]))

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef unsigned long long s64;
typedef u32 b32;

typedef struct time
{
    u64 Clock;
    u64 Counter;
} time;

typedef struct memory_operation memory_operation;
typedef void memory_operation_handler(memory_operation *);

typedef struct memory_pattern
{
    u8 *Source;
    s64 SourceStride;
    u64 SourceMask;
    
    u8 *Dest;
    s64 DestStride;
    u64 DestMask;
} memory_pattern;

struct memory_operation
{
    //
    // NOTE(casey): Input
    //
    
    memory_pattern Pattern;
    
    u64 Count;
    u8 *Values;
    
    memory_operation_handler *Handler;
    u64 StartGateCounter;
    
    //
    // NOTE(casey): Output
    //
    
    time StartStamp;
    time EndStamp;
    
    // NOTE(casey): Because threads write back into their operations, care must be taken to ensure memory_operation
    // structures exactly fill cache lines.
    u64 Pad[2];
};
CTAssert(SizeOf(memory_operation) == 128);

typedef struct time_stat
{
    time Min;
    time Max;
    time Sum;
    u32 Count;
} time_stat;

typedef struct memory_test_results
{
    char Name[256];
    
    time_stat Total;
    time_stat Thread;
    
    u64 TotalSize;
    u32 ThreadCount;
    u32 HandlerIndex;
} memory_test_results;

#define HANDLER_ENTRY(Function) {Function, #Function}
typedef struct handler_table_entry
{
    memory_operation_handler *Function;
    char *Name;
} handler_table_entry;

typedef struct context
{
    time BaseHz;
    u32 MaxThreadCount;
    
    u32 HandlerCount;
    handler_table_entry *Handlers;
    
    char *CPUBrand;
    u32 LogicalCoreCount;
} context;

// NOTE(casey): The platform-specific code provides these functions
function void DispatchWork(context *Context, u32 OpCount, memory_operation *Ops);
function memory_operation *ReceiveWorkResult(context *Context);
function void Statusf(char const *Format, ...);
function void Dataf(char const *Format, ...);
function void *AllocateAndClear(u64 Size);
