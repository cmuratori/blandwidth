/* ========================================================================
   $File: work/tools/blandwidth/blandwidth.c $
   $Date: 2020/06/17 05:05:33 UTC $
   $Revision: 5 $
   $Creator: Casey Muratori $
   ======================================================================== */

function u64
Minimum(u64 A, u64 B)
{
    u64 Result = (A < B) ? A : B;
    return(Result);
}

function u64
Maximum(u64 A, u64 B)
{
    u64 Result = (A > B) ? A : B;
    return(Result);
}

function u64
RoundedDiv(u64 A, u64 B)
{
    u64 Result = A;
    if(B)
    {
        Result = (A + B/2) / B;
    }
    
    return(Result);
}

function void
NoOp(memory_operation *Op)
{
    (void)(Op);
}

function void *
AllocateAndFill(u64 Size)
{
    u8 *Result = (u8 *)AllocateAndClear(Size);
    for(u64 ValueIndex = 0;
        ValueIndex < Size;
        ++ValueIndex)
    {
        Result[ValueIndex] = (255 - (u8)ValueIndex);
    }
    
    return(Result);
}
    
function timestamp
Subtract(timestamp A, timestamp B)
{
    timestamp Result;
    
    Result.Clock = A.Clock - B.Clock;
    Result.Counter = A.Counter - B.Counter;
    
    return(Result);
}

function timestamp
Average(time_stat A)
{
    timestamp Result = A.Sum;

    if(A.Count)
    {
        Result.Clock /= A.Count;
        Result.Counter /= A.Count;
    }
    
    return(Result);
}

function void
Include(time_stat *Stat, timestamp T)
{
    if(Stat->Count)
    {
        Stat->Min.Clock = Minimum(Stat->Min.Clock, T.Clock);
        Stat->Min.Counter = Minimum(Stat->Min.Counter, T.Counter);
        Stat->Max.Clock = Maximum(Stat->Max.Clock, T.Clock);
        Stat->Max.Counter = Maximum(Stat->Max.Counter, T.Counter);
    }
    else
    {
        Stat->Min = T;
        Stat->Max = T;
    }
    
    Stat->Sum.Clock += T.Clock;
    Stat->Sum.Counter += T.Counter;
    ++Stat->Count;
}

function u64
GetNanoseconds(timestamp BaseHz, u64 A)
{
    u64 NSPerS = 1000ULL * 1000 * 1000;
    u64 Result = (NSPerS*A)/BaseHz.Counter;
    return(Result);
}

function u64
GetBandwidthAs(timestamp BaseHz, memory_test_results *Res, u64 Unit)
{
    u64 Hz = BaseHz.Counter;
    u64 Measure = Res->Total.Min.Counter;
    
    u64 BytesPerSecond = 0;
    if(Measure)
    {
        BytesPerSecond = (Res->TotalSize * Hz) / Measure;
    }
    
    u64 Result = RoundedDiv(BytesPerSecond, Unit);
    
    return(Result);
}

function u64
GetBandwidth(timestamp BaseHz, memory_test_results *Res)
{
    u64 Result = GetBandwidthAs(BaseHz, Res, 1);
    return(Result);
}

function void
TimeOperation(context *Context, u32 OpCount, memory_operation *Operations, time_stat *ThreadStat, time_stat *TotalStat)
{
    u64 MaxCyclesToSpend = (1ULL*1000*1000*1000);
    u64 CyclesSpentOnNewMin = 0;
    while(CyclesSpentOnNewMin < MaxCyclesToSpend)
    {
        timestamp Now;
        TIME_OPEN(Now);
        u64 StartGate = (Now.Counter + Context->BaseHz.Counter)/1000;
        for(u32 OpIndex = 0;
            OpIndex < OpCount;
            ++OpIndex)
        {
            Operations[OpIndex].StartGateCounter = StartGate;
        }

        DispatchWork(Context, OpCount, Operations);
        
        time_stat ThisRun = {0};
        for(u32 ThreadIndex = 0;
            ThreadIndex < OpCount;
            ++ThreadIndex)
        {
            memory_operation *ResultOp = ReceiveWorkResult(Context);
            timestamp ThreadTime = Subtract(ResultOp->EndStamp, ResultOp->StartStamp);
            Include(ThreadStat, ThreadTime);
            Include(&ThisRun, ResultOp->EndStamp);
            Include(&ThisRun, ResultOp->StartStamp);
        }
        
        timestamp TotalTime = Subtract(ThisRun.Max, ThisRun.Min);
        CyclesSpentOnNewMin += TotalTime.Clock;
        
        timestamp PrevMin = TotalStat->Min;
        Include(TotalStat, TotalTime);
        if((TotalStat->Min.Clock != PrevMin.Clock) ||
           (TotalStat->Min.Counter != PrevMin.Counter))
        {
            // NOTE(casey): Every time we see a new minimum clock or counter, restart the testing
            CyclesSpentOnNewMin = 0;
        }
    }
}

function void
Main(context *Context)
{
    u64 Megabyte = 1024*1024;
    u64 Gigabyte = 1024*Megabyte;
    u64 Million = 1000*1000;
    
    //
    // NOTE(casey): Print the informational header
    //
    
    Statusf("\n");
    Statusf("========================================================================\n");
    Statusf("BlandWidth " VERSION_STRING " - A compact bandwidth tester for x64 CPUs\n");
    Statusf("by Casey Muratori circa 2020\n");
    Statusf("========================================================================\n");
    Statusf("\n");
    Statusf("WARNING: USE AT YOUR OWN RISK.  Numbers reported by this utility reflect\n");
    Statusf("the interplay of cores, clocking, hyperthreading, buffers, caches, and\n");
    Statusf("memory. Considerable expertise is required to interpret them properly.\n");
    Statusf("\n");
    
    Statusf("CPU: %s\n", Context->CPUBrand);
    Statusf("Logical Cores: %u\n", Context->LogicalCoreCount);
    Statusf("Expected frequency: %Iumhz\n", RoundedDiv(Context->BaseHz.Clock, Million));
    Statusf("Support: ");
    for(u32 HandlerIndex = 0;
        HandlerIndex < Context->HandlerCount;
        ++HandlerIndex)
    {
        Statusf(" %s", Context->Handlers[HandlerIndex].Name);
    }
    Statusf("\n\n");
    
    //
    // NOTE(casey): Prepare test buffers
    //
    
    memory_operation *Operations = (memory_operation *)AllocateAndClear(Context->MaxThreadCount*SizeOf(memory_operation));
    
    u64 BankSize = 4ULL*Gigabyte;
    u8 *Bank[2];
    Bank[0] = (u8 *)AllocateAndFill(BankSize);
    Bank[1] = (u8 *)AllocateAndFill(BankSize);

    u64 BankPerThread = BankSize / Context->MaxThreadCount;
    
    u32 ValuesSize = Context->MaxThreadCount*BLOCK_SIZE;
    u8 *ValuesBuffer = (u8 *)AllocateAndFill(ValuesSize);
    
    //
    // NOTE(casey): Run tests
    //
    
    u32 SizeOffset = 14;
    u32 SizeCount = 14;
    u32 MinThreadCount = 1;
    u32 ResultsPerTest = Context->HandlerCount*(Context->MaxThreadCount - MinThreadCount + 1);
    u32 ResultCount = SizeCount*ResultsPerTest;
    memory_test_results *TestResults = (memory_test_results *)AllocateAndClear(ResultCount*SizeOf(memory_test_results));
    
    u32 SourceBankIndex = 0;
    u32 DestBankIndex = 0;
    
    u32 ResultIndex = 0;
    for(u32 HandlerIndex = 0;
        HandlerIndex < Context->HandlerCount;
        ++HandlerIndex)
    {
        for(u32 SizeIndex = 0;
            SizeIndex < SizeCount;
            ++SizeIndex)
        {
            u64 TotalRegionSize = (1ULL << (SizeOffset + SizeIndex));
            u64 TotalRegionMask = (TotalRegionSize - 1);
            
            u32 FirstResultIndex = ResultIndex;
            memory_test_results *Fastest = TestResults + FirstResultIndex;
            memory_test_results *Slowest = TestResults + FirstResultIndex;
            for(u32 ThreadCount = MinThreadCount;
                ThreadCount <= Context->MaxThreadCount;
                ++ThreadCount)
            {
                memory_test_results *Results = TestResults + ResultIndex++;
                Results->ThreadCount = ThreadCount;
                Results->HandlerIndex = HandlerIndex;

                char *PrintSizeTable[] = {"b", "kb", "mb", "gb", 0};
                u32 PrintSizePower = 0;
                u64 PrintSize = TotalRegionSize;
                while(PrintSizeTable[PrintSizePower + 1] && (PrintSize > 1024))
                {
                    PrintSize /= 1024;
                    ++PrintSizePower;
                }
                
                Stringf(Results->Name, "%s %Iu%s/%ut", Context->Handlers[HandlerIndex].Name, PrintSize, PrintSizeTable[PrintSizePower], ThreadCount);
                
                Results->TotalSize = 0;
                for(u32 ThreadIndex = 0;
                    ThreadIndex < ThreadCount;
                    ++ThreadIndex)
                {
                    memory_operation *ThreadOp = Operations + ThreadIndex;
                    
                    ThreadOp->Pattern.Source = Bank[SourceBankIndex] + ThreadIndex*BankPerThread;
                    ThreadOp->Pattern.Dest = Bank[DestBankIndex] + ThreadIndex*BankPerThread;
                    
                    ThreadOp->Pattern.SourceStride = BLOCK_SIZE;
                    ThreadOp->Pattern.SourceMask = TotalRegionMask;
                    
                    ThreadOp->Pattern.DestStride = BLOCK_SIZE;
                    ThreadOp->Pattern.DestMask = TotalRegionMask;
                    
                    ThreadOp->Count = (4*TotalRegionSize) / BLOCK_SIZE;
                    if(ThreadOp->Count < Million)
                    {
                        ThreadOp->Count = Million;
                    }
                    
                    ThreadOp->Values = ValuesBuffer + ThreadIndex*BLOCK_SIZE;
                    
                    Results->TotalSize += ThreadOp->Count*BLOCK_SIZE;
                    
                    ThreadOp->Handler = Context->Handlers[HandlerIndex].Function;
                }
                
                TimeOperation(Context, ThreadCount, Operations, &Results->Thread, &Results->Total);
                
                if(GetBandwidth(Context->BaseHz, Fastest) < GetBandwidth(Context->BaseHz, Results))
                {
                    Fastest = Results;
                }
                
                if(GetBandwidth(Context->BaseHz, Slowest) > GetBandwidth(Context->BaseHz, Results))
                {
                    Slowest = Results;
                }

                Statusf("\r%s %IuGB/s (best: %IuGB/s)    ", 
                        Results->Name, 
                        GetBandwidthAs(Context->BaseHz, Results, Gigabyte),
                        GetBandwidthAs(Context->BaseHz, Fastest, Gigabyte));
            }
            
            Statusf("\rBest: %s %IuGB/s (slowest: %ut - %IuGB/s)\n",
                    Fastest->Name, GetBandwidthAs(Context->BaseHz, Fastest, Gigabyte),
                    Slowest->ThreadCount, GetBandwidthAs(Context->BaseHz, Slowest, Gigabyte));
        }
    }
    
    Statusf("\n\n");
    
    Dataf("Test,Estimated MB/s,Threads,Handler Index,Size,Min Total ns,Max Total ns,Avg Total ns,Min Total Clocks,Max Total Clocks,Avg Total Clocks,Min Thread ns,Max Thread ns,Avg Thread ns,Min Thread Clocks,Max Thread Clocks,Avg Thread Clocks\n");
    for(ResultIndex = 0;
        ResultIndex < ResultCount;
        ++ResultIndex)
    {
        memory_test_results *Result = TestResults + ResultIndex;
        if(Result->Total.Count)
        {
            timestamp TotalAvg = Average(Result->Total);
            timestamp ThreadAvg = Average(Result->Thread);
            
            u64 MinTotalNS = GetNanoseconds(Context->BaseHz, Result->Total.Min.Counter);
            u64 MaxTotalNS = GetNanoseconds(Context->BaseHz, Result->Total.Max.Counter);
            u64 AvgTotalNS = GetNanoseconds(Context->BaseHz, TotalAvg.Counter);
            
            u64 MinTotalClocks = Result->Total.Min.Clock;
            u64 MaxTotalClocks = Result->Total.Max.Clock;
            u64 AvgTotalClocks = TotalAvg.Clock;
            
            u64 MinThreadNS = GetNanoseconds(Context->BaseHz, Result->Thread.Min.Counter);
            u64 MaxThreadNS = GetNanoseconds(Context->BaseHz, Result->Thread.Max.Counter);
            u64 AvgThreadNS = GetNanoseconds(Context->BaseHz, ThreadAvg.Counter);
            
            u64 MinThreadClocks = Result->Thread.Min.Clock;
            u64 MaxThreadClocks = Result->Thread.Max.Clock;
            u64 AvgThreadClocks = ThreadAvg.Clock;
            
            Dataf("%s,%Iu,%u,%u,%Iu,%Iu,%Iu,%Iu,%Iu,%Iu,%Iu,%Iu,%Iu,%Iu,%Iu,%Iu,%Iu\n",
                  Result->Name,
                  GetBandwidthAs(Context->BaseHz, Result, Megabyte),
                  Result->ThreadCount,
                  Result->HandlerIndex,
                  Result->TotalSize,
                  MinTotalNS, MaxTotalNS, AvgTotalNS,
                  MinTotalClocks, MaxTotalClocks, AvgTotalClocks,
                  MinThreadNS, MaxThreadNS, AvgThreadNS,
                  MinThreadClocks, MaxThreadClocks, AvgThreadClocks);
        }
    }
}
