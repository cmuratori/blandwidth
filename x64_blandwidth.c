/* ========================================================================
   $File: work/tools/blandwidth/x64_blandwidth.c $
   $Date: 2020/06/16 21:46:28 UTC $
   $Revision: 1 $
   $Creator: Casey Muratori $
   ======================================================================== */

// NOTE(keeba): This is used instead of _mm256_xor_si256 because _mm256_xor_ps does not require AVX2.
#define Xor256(A, B) _mm256_castps_si256(_mm256_xor_ps(_mm256_castsi256_ps(A), _mm256_castsi256_ps(B)))

function void
X64Read128(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (8*SizeOf(__m128i)));
    
    memory_pattern Pattern = Op->Pattern;
    __m128i *Values = (__m128i *)Op->Values;
    u64 SourceOffset = 0;
    u64 Count = Op->Count;
    
    __m128i V0 = _mm_loadu_si128(Values + 0);
    __m128i V1 = _mm_loadu_si128(Values + 1);
    __m128i V2 = _mm_loadu_si128(Values + 2);
    __m128i V3 = _mm_loadu_si128(Values + 3);
    __m128i V4 = _mm_loadu_si128(Values + 4);
    __m128i V5 = _mm_loadu_si128(Values + 5);
    __m128i V6 = _mm_loadu_si128(Values + 6);
    __m128i V7 = _mm_loadu_si128(Values + 7);
    
    while(Count--)
    {
        MCA_BEGIN(Read128);
        
        __m128i *Source = (__m128i *)(Pattern.Source + SourceOffset);
        
        __m128i L0 = _mm_loadu_si128(Source + 0);
        __m128i L1 = _mm_loadu_si128(Source + 1);
        __m128i L2 = _mm_loadu_si128(Source + 2);
        __m128i L3 = _mm_loadu_si128(Source + 3);
        __m128i L4 = _mm_loadu_si128(Source + 4);
        __m128i L5 = _mm_loadu_si128(Source + 5);
        __m128i L6 = _mm_loadu_si128(Source + 6);
        __m128i L7 = _mm_loadu_si128(Source + 7);
        
        V0 = _mm_xor_si128(V0, L0);
        V1 = _mm_xor_si128(V1, L1);
        V2 = _mm_xor_si128(V2, L2);
        V3 = _mm_xor_si128(V3, L3);
        V4 = _mm_xor_si128(V4, L4);
        V5 = _mm_xor_si128(V5, L5);
        V6 = _mm_xor_si128(V6, L6);
        V7 = _mm_xor_si128(V7, L7);
        
        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
    }
        
    MCA_END(Read128);
    
    _mm_storeu_si128(Values + 0, V0);
    _mm_storeu_si128(Values + 1, V1);
    _mm_storeu_si128(Values + 2, V2);
    _mm_storeu_si128(Values + 3, V3);
    _mm_storeu_si128(Values + 4, V4);
    _mm_storeu_si128(Values + 5, V5);
    _mm_storeu_si128(Values + 6, V6);
    _mm_storeu_si128(Values + 7, V7);
}

function void
X64Write128(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (8*SizeOf(__m128i)));
    
    memory_pattern Pattern = Op->Pattern;
    __m128i *Values = (__m128i *)Op->Values;
    u64 DestOffset = 0;
    u64 Count = Op->Count;
    
    __m128i V0 = _mm_loadu_si128(Values + 0);
    __m128i V1 = _mm_loadu_si128(Values + 1);
    __m128i V2 = _mm_loadu_si128(Values + 2);
    __m128i V3 = _mm_loadu_si128(Values + 3);
    __m128i V4 = _mm_loadu_si128(Values + 4);
    __m128i V5 = _mm_loadu_si128(Values + 5);
    __m128i V6 = _mm_loadu_si128(Values + 6);
    __m128i V7 = _mm_loadu_si128(Values + 7);

    while(Count--)
    {
        MCA_BEGIN(Write128);
    
        __m128i *Dest = (__m128i *)(Pattern.Dest + DestOffset);
        
        V0 = _mm_xor_si128(V0, V1);
        V1 = _mm_xor_si128(V1, V2);
        V2 = _mm_xor_si128(V2, V3);
        V3 = _mm_xor_si128(V3, V4);
        V4 = _mm_xor_si128(V4, V5);
        V5 = _mm_xor_si128(V5, V6);
        V6 = _mm_xor_si128(V6, V7);
        V7 = _mm_xor_si128(V7, V0);
        
        _mm_storeu_si128(Dest + 0, V0);
        _mm_storeu_si128(Dest + 1, V1);
        _mm_storeu_si128(Dest + 2, V2);
        _mm_storeu_si128(Dest + 3, V3);
        _mm_storeu_si128(Dest + 4, V4);
        _mm_storeu_si128(Dest + 5, V5);
        _mm_storeu_si128(Dest + 6, V6);
        _mm_storeu_si128(Dest + 7, V7);
        
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    MCA_END(Write128);
    
    _mm_storeu_si128(Values + 0, V0);
    _mm_storeu_si128(Values + 1, V1);
    _mm_storeu_si128(Values + 2, V2);
    _mm_storeu_si128(Values + 3, V3);
    _mm_storeu_si128(Values + 4, V4);
    _mm_storeu_si128(Values + 5, V5);
    _mm_storeu_si128(Values + 6, V6);
    _mm_storeu_si128(Values + 7, V7);
}

function void
X64ReadWrite128(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (8*SizeOf(__m128i)));
    
    memory_pattern Pattern = Op->Pattern;
    __m128i *Values = (__m128i *)Op->Values;
    u64 SourceOffset = 0;
    u64 DestOffset = 0;
    u64 Count = Op->Count;
    
    __m128i V0 = _mm_loadu_si128(Values + 0);
    __m128i V1 = _mm_loadu_si128(Values + 1);
    __m128i V2 = _mm_loadu_si128(Values + 2);
    __m128i V3 = _mm_loadu_si128(Values + 3);
    __m128i V4 = _mm_loadu_si128(Values + 4);
    __m128i V5 = _mm_loadu_si128(Values + 5);
    __m128i V6 = _mm_loadu_si128(Values + 6);
    __m128i V7 = _mm_loadu_si128(Values + 7);
    
    while(Count--)
    {
        MCA_BEGIN(ReadWrite128);
        
        __m128i *Source = (__m128i *)(Pattern.Source + SourceOffset);
        __m128i *Dest = (__m128i *)(Pattern.Dest + DestOffset);

        __m128i L0 = _mm_loadu_si128(Source + 0);
        __m128i L1 = _mm_loadu_si128(Source + 1);
        __m128i L2 = _mm_loadu_si128(Source + 2);
        __m128i L3 = _mm_loadu_si128(Source + 3);

        V0 = _mm_xor_si128(V0, L0);
        V1 = _mm_xor_si128(V1, L1);
        V2 = _mm_xor_si128(V2, L2);
        V3 = _mm_xor_si128(V3, L3);
        
        _mm_storeu_si128(Dest + 0, V0);
        _mm_storeu_si128(Dest + 1, V1);
        _mm_storeu_si128(Dest + 2, V2);
        _mm_storeu_si128(Dest + 3, V3);
        
        __m128i L4 = _mm_loadu_si128(Source + 4);
        __m128i L5 = _mm_loadu_si128(Source + 5);
        __m128i L6 = _mm_loadu_si128(Source + 6);
        __m128i L7 = _mm_loadu_si128(Source + 7);
        
        V4 = _mm_xor_si128(V4, L4);
        V5 = _mm_xor_si128(V5, L5);
        V6 = _mm_xor_si128(V6, L6);
        V7 = _mm_xor_si128(V7, L7);
        
        _mm_storeu_si128(Dest + 4, V4);
        _mm_storeu_si128(Dest + 5, V5);
        _mm_storeu_si128(Dest + 6, V6);
        _mm_storeu_si128(Dest + 7, V7);
        
        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    MCA_END(ReadWrite128);
    
    _mm_storeu_si128(Values + 0, V0);
    _mm_storeu_si128(Values + 1, V1);
    _mm_storeu_si128(Values + 2, V2);
    _mm_storeu_si128(Values + 3, V3);
    _mm_storeu_si128(Values + 4, V4);
    _mm_storeu_si128(Values + 5, V5);
    _mm_storeu_si128(Values + 6, V6);
    _mm_storeu_si128(Values + 7, V7);
}

function_avx2 void
X64Read256(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (4*SizeOf(__m256i)));
    
    memory_pattern Pattern = Op->Pattern;
    __m256i *Values = (__m256i *)Op->Values;
    u64 SourceOffset = 0;
    u64 Count = Op->Count;

    __m256i V0 = _mm256_loadu_si256(Values + 0);
    __m256i V1 = _mm256_loadu_si256(Values + 1);
    __m256i V2 = _mm256_loadu_si256(Values + 2);
    __m256i V3 = _mm256_loadu_si256(Values + 3);
    
    while(Count--)
    {
        MCA_BEGIN(Read256);

        __m256i *Source = (__m256i *)(Pattern.Source + SourceOffset);

        __m256i L0 = _mm256_loadu_si256(Source + 0);
        __m256i L1 = _mm256_loadu_si256(Source + 1);
        __m256i L2 = _mm256_loadu_si256(Source + 2);
        __m256i L3 = _mm256_loadu_si256(Source + 3);

        V0 = Xor256(V0, L0);
        V1 = Xor256(V1, L1);
        V2 = Xor256(V2, L2);
        V3 = Xor256(V3, L3);

        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
    }
    MCA_END(Read256);
    
    _mm256_storeu_si256(Values + 0, V0);
    _mm256_storeu_si256(Values + 1, V1);
    _mm256_storeu_si256(Values + 2, V2);
    _mm256_storeu_si256(Values + 3, V3);
}

function_avx2 void
X64Write256(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (4*SizeOf(__m256i)));
    
    memory_pattern Pattern = Op->Pattern;
    __m256i *Values = (__m256i *)Op->Values;
    u64 DestOffset = 0;
    u64 Count = Op->Count;

    __m256i V0 = _mm256_loadu_si256(Values + 0);
    __m256i V1 = _mm256_loadu_si256(Values + 1);
    __m256i V2 = _mm256_loadu_si256(Values + 2);
    __m256i V3 = _mm256_loadu_si256(Values + 3);
    
    while(Count--)
    {
        MCA_BEGIN(Write256);

        __m256i *Dest = (__m256i *)(Pattern.Dest + DestOffset);

        V0 = Xor256(V0, V1);
        V1 = Xor256(V1, V2);
        V2 = Xor256(V2, V3);
        V3 = Xor256(V3, V0);
        
        _mm256_storeu_si256(Dest + 0, V0);
        _mm256_storeu_si256(Dest + 1, V1);
        _mm256_storeu_si256(Dest + 2, V2);
        _mm256_storeu_si256(Dest + 3, V3);
        
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    MCA_END(Write256);
        
    _mm256_storeu_si256(Values + 0, V0);
    _mm256_storeu_si256(Values + 1, V1);
    _mm256_storeu_si256(Values + 2, V2);
    _mm256_storeu_si256(Values + 3, V3);
}

function_avx2 void
X64ReadWrite256(memory_operation *Op)
{
    CTAssert(BLOCK_SIZE == (4*SizeOf(__m256i)));

    memory_pattern Pattern = Op->Pattern;
    __m256i *Values = (__m256i *)Op->Values;
    u64 SourceOffset = 0;
    u64 DestOffset = 0;
    u64 Count = Op->Count;
    
    __m256i V0 = _mm256_loadu_si256(Values + 0);
    __m256i V1 = _mm256_loadu_si256(Values + 1);
    __m256i V2 = _mm256_loadu_si256(Values + 2);
    __m256i V3 = _mm256_loadu_si256(Values + 3);
    
    while(Count--)
    {
        MCA_BEGIN(ReadWrite256);

        __m256i *Source = (__m256i *)(Pattern.Source + SourceOffset);
        __m256i *Dest = (__m256i *)(Pattern.Dest + DestOffset);
        
        __m256i L0 = _mm256_loadu_si256(Source + 0);
        __m256i L1 = _mm256_loadu_si256(Source + 1);
        __m256i L2 = _mm256_loadu_si256(Source + 2);
        __m256i L3 = _mm256_loadu_si256(Source + 3);

        V0 = Xor256(V0, L0);
        V1 = Xor256(V1, L1);
        V2 = Xor256(V2, L2);
        V3 = Xor256(V3, L3);
        
        _mm256_storeu_si256(Dest + 0, V0);
        _mm256_storeu_si256(Dest + 1, V1);
        _mm256_storeu_si256(Dest + 2, V2);
        _mm256_storeu_si256(Dest + 3, V3);
        
        SourceOffset = (SourceOffset + Pattern.SourceStride) & Pattern.SourceMask;
        DestOffset = (DestOffset + Pattern.DestStride) & Pattern.DestMask;
    }
    MCA_END(ReadWrite256);
    
    _mm256_storeu_si256(Values + 0, V0);
    _mm256_storeu_si256(Values + 1, V1);
    _mm256_storeu_si256(Values + 2, V2);
    _mm256_storeu_si256(Values + 3, V3);
}

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
