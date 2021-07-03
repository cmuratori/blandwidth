/* ========================================================================
   $File: work/tools/blandwidth/x64_blandwidth.c $
   $Date: 2020/06/16 21:46:28 UTC $
   $Revision: 3 $
   $Creator: Casey Muratori $
   ======================================================================== */

function void
X64Read128(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (8*SizeOf(__m128)));
    
    memory_pattern Pattern = Op->Pattern;
    f32 *Values = (f32 *)Op->Values;
    u64 SourceOffset = 0;
    u64 Count = Op->Count;
    
    __m128 V0 = _mm_loadu_ps(Values + 0*4);
    __m128 V1 = _mm_loadu_ps(Values + 1*4);
    __m128 V2 = _mm_loadu_ps(Values + 2*4);
    __m128 V3 = _mm_loadu_ps(Values + 3*4);
    __m128 V4 = _mm_loadu_ps(Values + 4*4);
    __m128 V5 = _mm_loadu_ps(Values + 5*4);
    __m128 V6 = _mm_loadu_ps(Values + 6*4);
    __m128 V7 = _mm_loadu_ps(Values + 7*4);
    
    while(Count--)
    {
        MCA_BEGIN(Read128);
        
        f32 *Source = (f32 *)(Pattern.Source + SourceOffset);
        
        __m128 L0 = _mm_loadu_ps(Source + 0*4);
        __m128 L1 = _mm_loadu_ps(Source + 1*4);
        __m128 L2 = _mm_loadu_ps(Source + 2*4);
        __m128 L3 = _mm_loadu_ps(Source + 3*4);
        __m128 L4 = _mm_loadu_ps(Source + 4*4);
        __m128 L5 = _mm_loadu_ps(Source + 5*4);
        __m128 L6 = _mm_loadu_ps(Source + 6*4);
        __m128 L7 = _mm_loadu_ps(Source + 7*4);
        
        V0 = _mm_xor_ps(V0, L0);
        V1 = _mm_xor_ps(V1, L1);
        V2 = _mm_xor_ps(V2, L2);
        V3 = _mm_xor_ps(V3, L3);
        V4 = _mm_xor_ps(V4, L4);
        V5 = _mm_xor_ps(V5, L5);
        V6 = _mm_xor_ps(V6, L6);
        V7 = _mm_xor_ps(V7, L7);
        
        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
    }
        
    MCA_END(Read128);
    
    _mm_storeu_ps(Values + 0*4, V0);
    _mm_storeu_ps(Values + 1*4, V1);
    _mm_storeu_ps(Values + 2*4, V2);
    _mm_storeu_ps(Values + 3*4, V3);
    _mm_storeu_ps(Values + 4*4, V4);
    _mm_storeu_ps(Values + 5*4, V5);
    _mm_storeu_ps(Values + 6*4, V6);
    _mm_storeu_ps(Values + 7*4, V7);
}

function void
X64Write128(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (8*SizeOf(__m128)));
    
    memory_pattern Pattern = Op->Pattern;
    f32 *Values = (f32 *)Op->Values;
    u64 DestOffset = 0;
    u64 Count = Op->Count;
    
    __m128 V0 = _mm_loadu_ps(Values + 0*4);
    __m128 V1 = _mm_loadu_ps(Values + 1*4);
    __m128 V2 = _mm_loadu_ps(Values + 2*4);
    __m128 V3 = _mm_loadu_ps(Values + 3*4);
    __m128 V4 = _mm_loadu_ps(Values + 4*4);
    __m128 V5 = _mm_loadu_ps(Values + 5*4);
    __m128 V6 = _mm_loadu_ps(Values + 6*4);
    __m128 V7 = _mm_loadu_ps(Values + 7*4);

    while(Count--)
    {
        MCA_BEGIN(Write128);
        
        f32 *Dest = (f32 *)(Pattern.Dest + DestOffset);
        
        V0 = _mm_xor_ps(V0, V1);
        V1 = _mm_xor_ps(V1, V2);
        V2 = _mm_xor_ps(V2, V3);
        V3 = _mm_xor_ps(V3, V4);
        V4 = _mm_xor_ps(V4, V5);
        V5 = _mm_xor_ps(V5, V6);
        V6 = _mm_xor_ps(V6, V7);
        V7 = _mm_xor_ps(V7, V0);
        
        _mm_storeu_ps(Dest + 0*4, V0);
        _mm_storeu_ps(Dest + 1*4, V1);
        _mm_storeu_ps(Dest + 2*4, V2);
        _mm_storeu_ps(Dest + 3*4, V3);
        _mm_storeu_ps(Dest + 4*4, V4);
        _mm_storeu_ps(Dest + 5*4, V5);
        _mm_storeu_ps(Dest + 6*4, V6);
        _mm_storeu_ps(Dest + 7*4, V7);
        
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    MCA_END(Write128);
    
    _mm_storeu_ps(Values + 0*4, V0);
    _mm_storeu_ps(Values + 1*4, V1);
    _mm_storeu_ps(Values + 2*4, V2);
    _mm_storeu_ps(Values + 3*4, V3);
    _mm_storeu_ps(Values + 4*4, V4);
    _mm_storeu_ps(Values + 5*4, V5);
    _mm_storeu_ps(Values + 6*4, V6);
    _mm_storeu_ps(Values + 7*4, V7);
}

function void
X64ReadWrite128(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (8*SizeOf(__m128)));
    
    memory_pattern Pattern = Op->Pattern;
    f32 *Values = (f32 *)Op->Values;
    u64 SourceOffset = 0;
    u64 DestOffset = 0;
    u64 Count = Op->Count;
    
    __m128 V0 = _mm_loadu_ps(Values + 0*4);
    __m128 V1 = _mm_loadu_ps(Values + 1*4);
    __m128 V2 = _mm_loadu_ps(Values + 2*4);
    __m128 V3 = _mm_loadu_ps(Values + 3*4);
    __m128 V4 = _mm_loadu_ps(Values + 4*4);
    __m128 V5 = _mm_loadu_ps(Values + 5*4);
    __m128 V6 = _mm_loadu_ps(Values + 6*4);
    __m128 V7 = _mm_loadu_ps(Values + 7*4);
    
    while(Count--)
    {
        MCA_BEGIN(ReadWrite128);
        
        f32 *Source = (f32 *)(Pattern.Source + SourceOffset);
        f32 *Dest = (f32 *)(Pattern.Dest + DestOffset);
        
        __m128 L0 = _mm_loadu_ps(Source + 0*4);
        __m128 L1 = _mm_loadu_ps(Source + 1*4);
        __m128 L2 = _mm_loadu_ps(Source + 2*4);
        __m128 L3 = _mm_loadu_ps(Source + 3*4);
        
        V0 = _mm_xor_ps(V0, L0);
        V1 = _mm_xor_ps(V1, L1);
        V2 = _mm_xor_ps(V2, L2);
        V3 = _mm_xor_ps(V3, L3);
        
        _mm_storeu_ps(Dest + 0*4, V0);
        _mm_storeu_ps(Dest + 1*4, V1);
        _mm_storeu_ps(Dest + 2*4, V2);
        _mm_storeu_ps(Dest + 3*4, V3);
        
        __m128 L4 = _mm_loadu_ps(Source + 4*4);
        __m128 L5 = _mm_loadu_ps(Source + 5*4);
        __m128 L6 = _mm_loadu_ps(Source + 6*4);
        __m128 L7 = _mm_loadu_ps(Source + 7*4);
        
        V4 = _mm_xor_ps(V4, L4);
        V5 = _mm_xor_ps(V5, L5);
        V6 = _mm_xor_ps(V6, L6);
        V7 = _mm_xor_ps(V7, L7);
        
        _mm_storeu_ps(Dest + 4*4, V4);
        _mm_storeu_ps(Dest + 5*4, V5);
        _mm_storeu_ps(Dest + 6*4, V6);
        _mm_storeu_ps(Dest + 7*4, V7);
        
        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    MCA_END(ReadWrite128);
    
    _mm_storeu_ps(Values + 0*4, V0);
    _mm_storeu_ps(Values + 1*4, V1);
    _mm_storeu_ps(Values + 2*4, V2);
    _mm_storeu_ps(Values + 3*4, V3);
    _mm_storeu_ps(Values + 4*4, V4);
    _mm_storeu_ps(Values + 5*4, V5);
    _mm_storeu_ps(Values + 6*4, V6);
    _mm_storeu_ps(Values + 7*4, V7);
}

function_avx void
X64Read256(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (4*SizeOf(__m256)));
    
    memory_pattern Pattern = Op->Pattern;
    f32 *Values = (f32 *)Op->Values;
    u64 SourceOffset = 0;
    u64 Count = Op->Count;
    
    __m256 V0 = _mm256_loadu_ps(Values + 0*8);
    __m256 V1 = _mm256_loadu_ps(Values + 1*8);
    __m256 V2 = _mm256_loadu_ps(Values + 2*8);
    __m256 V3 = _mm256_loadu_ps(Values + 3*8);
    
    while(Count--)
    {
        MCA_BEGIN(Read256);

        f32 *Source = (f32 *)(Pattern.Source + SourceOffset);

        __m256 L0 = _mm256_loadu_ps(Source + 0*8);
        __m256 L1 = _mm256_loadu_ps(Source + 1*8);
        __m256 L2 = _mm256_loadu_ps(Source + 2*8);
        __m256 L3 = _mm256_loadu_ps(Source + 3*8);

        V0 = _mm256_xor_ps(V0, L0);
        V1 = _mm256_xor_ps(V1, L1);
        V2 = _mm256_xor_ps(V2, L2);
        V3 = _mm256_xor_ps(V3, L3);

        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
    }
    MCA_END(Read256);
    
    _mm256_storeu_ps(Values + 0*8, V0);
    _mm256_storeu_ps(Values + 1*8, V1);
    _mm256_storeu_ps(Values + 2*8, V2);
    _mm256_storeu_ps(Values + 3*8, V3);
}

function_avx void
X64Write256(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (4*SizeOf(__m256)));
    
    memory_pattern Pattern = Op->Pattern;
    f32 *Values = (f32 *)Op->Values;
    u64 DestOffset = 0;
    u64 Count = Op->Count;
    
    __m256 V0 = _mm256_loadu_ps(Values + 0*8);
    __m256 V1 = _mm256_loadu_ps(Values + 1*8);
    __m256 V2 = _mm256_loadu_ps(Values + 2*8);
    __m256 V3 = _mm256_loadu_ps(Values + 3*8);
    
    while(Count--)
    {
        MCA_BEGIN(Write256);
        
        f32 *Dest = (f32 *)(Pattern.Dest + DestOffset);
        
        V0 = _mm256_xor_ps(V0, V1);
        V1 = _mm256_xor_ps(V1, V2);
        V2 = _mm256_xor_ps(V2, V3);
        V3 = _mm256_xor_ps(V3, V0);
        
        _mm256_storeu_ps(Dest + 0*8, V0);
        _mm256_storeu_ps(Dest + 1*8, V1);
        _mm256_storeu_ps(Dest + 2*8, V2);
        _mm256_storeu_ps(Dest + 3*8, V3);
        
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    MCA_END(Write256);
    
    _mm256_storeu_ps(Values + 0*8, V0);
    _mm256_storeu_ps(Values + 1*8, V1);
    _mm256_storeu_ps(Values + 2*8, V2);
    _mm256_storeu_ps(Values + 3*8, V3);
}

function_avx void
X64ReadWrite256(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (4*SizeOf(__m256)));

    memory_pattern Pattern = Op->Pattern;
    f32 *Values = (f32 *)Op->Values;
    u64 SourceOffset = 0;
    u64 DestOffset = 0;
    u64 Count = Op->Count;
    
    __m256 V0 = _mm256_loadu_ps(Values + 0*8);
    __m256 V1 = _mm256_loadu_ps(Values + 1*8);
    __m256 V2 = _mm256_loadu_ps(Values + 2*8);
    __m256 V3 = _mm256_loadu_ps(Values + 3*8);
    
    while(Count--)
    {
        MCA_BEGIN(ReadWrite256);

        f32 *Source = (f32 *)(Pattern.Source + SourceOffset);
        f32 *Dest = (f32 *)(Pattern.Dest + DestOffset);
        
        __m256 L0 = _mm256_loadu_ps(Source + 0*8);
        __m256 L1 = _mm256_loadu_ps(Source + 1*8);
        __m256 L2 = _mm256_loadu_ps(Source + 2*8);
        __m256 L3 = _mm256_loadu_ps(Source + 3*8);
        
        V0 = _mm256_xor_ps(V0, L0);
        V1 = _mm256_xor_ps(V1, L1);
        V2 = _mm256_xor_ps(V2, L2);
        V3 = _mm256_xor_ps(V3, L3);
        
        _mm256_storeu_ps(Dest + 0*8, V0);
        _mm256_storeu_ps(Dest + 1*8, V1);
        _mm256_storeu_ps(Dest + 2*8, V2);
        _mm256_storeu_ps(Dest + 3*8, V3);
        
        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    MCA_END(ReadWrite256);
    
    _mm256_storeu_ps(Values + 0*8, V0);
    _mm256_storeu_ps(Values + 1*8, V1);
    _mm256_storeu_ps(Values + 2*8, V2);
    _mm256_storeu_ps(Values + 3*8, V3);
}

// NOTE(keeba): The X64*512 functions use _mm512_xor_si512 instead of _mm512_xor_ps because 512-bit vpxord is part
// of base AVX-512, while 512-bit vxorps is part of the AVX512DQ extension.
function_avx512 void
X64Read512(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (2*SizeOf(__m512i)));
    
    memory_pattern Pattern = Op->Pattern;
    __m512i *Values = (__m512i *)Op->Values;
    u64 SourceOffset = 0;
    u64 Count = Op->Count;
    
    __m512i V0 = _mm512_loadu_si512(Values + 0);
    __m512i V1 = _mm512_loadu_si512(Values + 1);
    
    while(Count--)
    {
        __m512i *Source = (__m512i *)(Pattern.Source + SourceOffset);
        
        __m512i L0 = _mm512_loadu_si512(Source + 0);
        __m512i L1 = _mm512_loadu_si512(Source + 1);
        
        V0 = _mm512_xor_si512(V0, L0);
        V1 = _mm512_xor_si512(V1, L1);
        
        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
    }
    
    _mm512_storeu_si512(Values + 0, V0);
    _mm512_storeu_si512(Values + 1, V1);
}

function_avx512 void
X64Write512(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (2*SizeOf(__m512i)));
    
    memory_pattern Pattern = Op->Pattern;
    __m512i *Values = (__m512i *)Op->Values;
    u64 DestOffset = 0;
    u64 Count = Op->Count;
    
    __m512i V0 = _mm512_loadu_si512(Values + 0);
    __m512i V1 = _mm512_loadu_si512(Values + 1);
    
    while(Count--)
    {
        __m512i *Dest = (__m512i *)(Pattern.Dest + DestOffset);
        
        V0 = _mm512_xor_si512(V0, V1);
        V1 = _mm512_xor_si512(V1, V0);
        
        _mm512_storeu_si512(Dest + 0, V0);
        _mm512_storeu_si512(Dest + 1, V1);
        
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    
    _mm512_storeu_si512(Values + 0, V0);
    _mm512_storeu_si512(Values + 1, V1);
}

function_avx512 void
X64ReadWrite512(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (2*SizeOf(__m512i)));

    memory_pattern Pattern = Op->Pattern;
    __m512i *Values = (__m512i *)Op->Values;
    u64 SourceOffset = 0;
    u64 DestOffset = 0;
    u64 Count = Op->Count;
    
    __m512i V0 = _mm512_loadu_si512(Values + 0);
    __m512i V1 = _mm512_loadu_si512(Values + 1);
    
    while(Count--)
    {
        __m512i *Source = (__m512i *)(Pattern.Source + SourceOffset);
        __m512i *Dest = (__m512i *)(Pattern.Dest + DestOffset);
    
        __m512i L0 = _mm512_loadu_si512(Source + 0);
        __m512i L1 = _mm512_loadu_si512(Source + 1);

        V0 = _mm512_xor_si512(V0, L0);
        V1 = _mm512_xor_si512(V1, L1);
        
        _mm512_storeu_si512(Dest + 0, V0);
        _mm512_storeu_si512(Dest + 1, V1);
        
        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    
    _mm512_storeu_si512(Values + 0, V0);
    _mm512_storeu_si512(Values + 1, V1);
}
