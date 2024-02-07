
//=================================
// Creating and editing
//=================================

external buffer
Buffer(void* Ptr, usz WriteCur, usz Size)
{
    buffer Result = { (u8*)Ptr, WriteCur, Size };
    return Result;
}

external void
ClearBuffer(buffer* Buf)
{
    if (Buf->Base)
    {
        memset(Buf->Base, 0, Buf->Size);
        Buf->WriteCur = 0;
    }
}

external void
AdvanceBuffer(buffer* Buf, usz NumBytes)
{
    Buf->Base += NumBytes;
    Buf->WriteCur -= NumBytes;
}

external bool
CopyData(void* Dst, usz DstSize, void* Src, usz SrcSize)
{
    if (DstSize >= SrcSize)
    {
        memcpy(Dst, Src, SrcSize);
        return true;
    }
    return false;
}

external bool
AppendDataToBuffer(void* Src, usz SrcSize, buffer* Dst)
{
    bool Result = CopyData(Dst->Base + Dst->WriteCur, Dst->Size - Dst->WriteCur, Src, SrcSize);
    if (Result)
    {
        Dst->WriteCur += SrcSize;
    }
    return Result;
}

external bool
AppendBufferToBuffer(buffer Src, buffer* Dst)
{
    bool Result = CopyData(Dst->Base + Dst->WriteCur, Dst->Size - Dst->WriteCur,
                           Src.Base, Src.WriteCur);
    if (Result)
    {
        Dst->WriteCur += Src.WriteCur;
    }
    return Result;
}

external bool
AppendBufferToBufferNTimes(buffer Src, usz Count, buffer* Dst)
{
    usz TotalSizeToWrite = Src.WriteCur * Count;
    usz RemainingSize = Dst->Size - Dst->WriteCur;
    if (TotalSizeToWrite <= RemainingSize)
    {
        while (Count--)
        {
            CopyData(Dst->Base + Dst->WriteCur, Dst->Size - Dst->WriteCur, Src.Base, Src.WriteCur);
            Dst->WriteCur += Src.WriteCur;
        }
        return true;
    }
    return false;
}

external void
ReplaceByteInBuffer(u8 OldByte, u8 NewByte, buffer Buffer)
{
    usz FoundIdx = 0;
    while ((FoundIdx = ByteInBuffer(OldByte, Buffer, RETURN_IDX_FIND)) != INVALID_IDX)
    {
        Buffer.Base[FoundIdx] = NewByte;
        AdvanceBuffer(&Buffer, FoundIdx);
    }
}

//=================================
// Query (Compare)
//=================================

internal usz
_ComparePtrSimple(void* A, void* B, usz AmountToCompare)
{
    u8* PtrA = (u8*)A;
    u8* PtrB = (u8*)B;
    for (usz Idx = 0; Idx < AmountToCompare; Idx++)
    {
        if (PtrA[Idx] != PtrB[Idx])
        {
            return (usz)(PtrA + Idx);
        }
    }
    return (usz)(PtrA + AmountToCompare);
}
internal usz (*_ComparePtr)(void*, void*, usz) = &_ComparePtrSimple;

internal usz
_CompareIdx(buffer A, buffer B, usz AmountToCompare)
{
    usz Result = _ComparePtr(A.Base, B.Base, AmountToCompare);
    return Result - (usz)A.Base;
}

internal bool
_CompareBool(buffer A, buffer B, usz AmountToCompare)
{
    usz Result = _CompareIdx(A, B, AmountToCompare);
    return Result == AmountToCompare;
}

external usz
CompareBuffers(buffer A, buffer B, usz AmountToCompare, int Flag)
{
    return (Flag & RETURN_BOOL ? _CompareBool(A, B, AmountToCompare)
            : Flag & RETURN_IDX_DIFF ? _CompareIdx(A, B, AmountToCompare)
            :                          _ComparePtr(A.Base, B.Base, AmountToCompare));
}

external bool
EqualBuffers(buffer A, buffer B)
{
    if (A.WriteCur == B.WriteCur)
    {
        if (A.WriteCur == 0) return true;
        else if (A.Base[0] == B.Base[0]
                 && A.Base[A.WriteCur-1] == B.Base[B.WriteCur-1]
                 && A.WriteCur == _CompareIdx(A, B, A.WriteCur)) return true;
    }
    return false;
}

//=================================
// Query (ByteInBuffer)
//=================================

internal usz
_ByteInBufferIdxSimple(u8 Needle, buffer Haystack)
{
    for (usz Idx = 0; Idx < Haystack.WriteCur; Idx++)
    {
        if (Needle == Haystack.Base[Idx])
        {
            return Idx;
        }
    }
    return INVALID_IDX;
}
internal usz (*_ByteInBufferIdx)(u8, buffer) = &_ByteInBufferIdxSimple;

internal usz
_ByteInBufferIdxFind(u8 Needle, buffer Haystack)
{
    usz Idx = _ByteInBufferIdx(Needle, Haystack);
    return Idx;
}

internal usz
_ByteInBufferIdxAfter(u8 Needle, buffer Haystack)
{
    usz Idx = _ByteInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? Idx + 1 : Idx;
    return Result;
}

internal usz
_ByteInBufferPtrFind(u8 Needle, buffer Haystack)
{
    usz Idx = _ByteInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? (usz)Haystack.Base + Idx : 0;
    return Result;
}

internal usz
_ByteInBufferPtrAfter(u8 Needle, buffer Haystack)
{
    usz Idx = _ByteInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? (usz)Haystack.Base + Idx + 1 : 0;
    return Result;
}

internal usz
_ByteInBufferBool(u8 Needle, buffer Haystack)
{
    usz Idx = _ByteInBufferIdx(Needle, Haystack);
    return Idx != INVALID_IDX;
}

internal usz
_ReverseByteInBufferIdxSimple(u8 Needle, buffer Haystack)
{
    u8* HaystackPtr = (u8*)Haystack.Base;
    if (Haystack.WriteCur > 0)
    {
        for (usz Idx = Haystack.WriteCur-1; Idx > 0; Idx--)
        {
            if (HaystackPtr[Idx] == Needle)
            {
                return Idx;
            }
        }
        
        if (HaystackPtr[0] == Needle)
        {
            return 0;
        }
    }
    return INVALID_IDX;
}
internal usz (*_ReverseByteInBufferIdx)(u8, buffer) = &_ReverseByteInBufferIdxSimple;

internal usz
_ReverseByteInBufferIdxFind(u8 Needle, buffer Haystack)
{
    usz Result = _ReverseByteInBufferIdx(Needle, Haystack);
    return Result;
}

internal usz
_ReverseByteInBufferIdxAfter(u8 Needle, buffer Haystack)
{
    usz Idx = _ReverseByteInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? Idx + 1 : Idx;
    return Result;
}

internal usz
_ReverseByteInBufferPtrFind(u8 Needle, buffer Haystack)
{
    usz Idx = _ReverseByteInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? (usz)Haystack.Base + Idx : 0;
    return Result;
}

internal usz
_ReverseByteInBufferPtrAfter(u8 Needle, buffer Haystack)
{
    usz Idx = _ReverseByteInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? (usz)Haystack.Base + Idx + 1 : 0;
    return Result;
}

internal usz
_ReverseByteInBufferBool(u8 Needle, buffer Haystack)
{
    usz Idx = _ReverseByteInBufferIdx(Needle, Haystack);
    return Idx != INVALID_IDX;
}

external usz
ByteInBuffer(u8 Needle, buffer Haystack, int Flags)
{
    return Flags & SEARCH_REVERSE ?
    (  Flags & RETURN_BOOL      ? _ReverseByteInBufferBool(Needle, Haystack)
     : Flags & RETURN_IDX_FIND  ? _ReverseByteInBufferIdxFind(Needle, Haystack)
     : Flags & RETURN_IDX_AFTER ? _ReverseByteInBufferIdxAfter(Needle, Haystack)
     : Flags & RETURN_PTR_AFTER ? _ReverseByteInBufferPtrAfter(Needle, Haystack)
     :                            _ReverseByteInBufferPtrFind(Needle, Haystack)) :
    (  Flags & RETURN_BOOL      ? _ByteInBufferBool(Needle, Haystack)
     : Flags & RETURN_IDX_FIND  ? _ByteInBufferIdxFind(Needle, Haystack)
     : Flags & RETURN_IDX_AFTER ? _ByteInBufferIdxAfter(Needle, Haystack)
     : Flags & RETURN_PTR_AFTER ? _ByteInBufferPtrAfter(Needle, Haystack)
     :                            _ByteInBufferPtrFind(Needle, Haystack));
}

//=================================
// Query (BufferInBuffer)
//=================================

internal usz
_BufferInBufferIdxSimple(buffer Needle, buffer Haystack)
{
    if (Haystack.WriteCur >= Needle.WriteCur)
    {
        u8* HaystackPtr = (u8*)Haystack.Base;
        u8* NeedlePtr = (u8*)Needle.Base;
        
        for (usz Idx = 0; Idx < Haystack.WriteCur - Needle.WriteCur + 1; Idx++)
        {
            if (HaystackPtr[Idx] == NeedlePtr[0]
                && HaystackPtr[Idx + Needle.WriteCur-1] == NeedlePtr[Needle.WriteCur-1]
                && EqualBuffers(Buffer(HaystackPtr + Idx, Needle.WriteCur, 0), Needle))
            {
                return Idx;
            }
        }
    }
    return INVALID_IDX;
}
internal usz (*_BufferInBufferIdx)(buffer, buffer) = &_BufferInBufferIdxSimple;

internal usz
_BufferInBufferIdxFind(buffer Needle, buffer Haystack)
{
    usz Result = _BufferInBufferIdx(Needle, Haystack);
    return Result;
}

internal usz
_BufferInBufferIdxAfter(buffer Needle, buffer Haystack)
{
    usz Idx = _BufferInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? Idx + Needle.WriteCur : Idx;
    return Result;
}

internal usz
_BufferInBufferPtrFind(buffer Needle, buffer Haystack)
{
    usz Idx = _BufferInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? (usz)Haystack.Base + Idx : 0;
    return Result;
}

internal usz
_BufferInBufferPtrAfter(buffer Needle, buffer Haystack)
{
    usz Idx = _BufferInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? (usz)Haystack.Base + Idx + Needle.WriteCur : 0;
    return Result;
}

internal usz
_BufferInBufferBool(buffer Needle, buffer Haystack)
{
    usz Idx = _BufferInBufferIdx(Needle, Haystack);
    return Idx != INVALID_IDX;
}

internal usz
_ReverseBufferInBufferIdxSimple(buffer Needle, buffer Haystack)
{
    if (Haystack.WriteCur >= Needle.WriteCur)
    {
        u8* HaystackPtr = (u8*)Haystack.Base;
        u8* NeedlePtr = (u8*)Needle.Base;
        
        for (usz Idx = Haystack.WriteCur - Needle.WriteCur; Idx > 0; Idx--)
        {
            if (HaystackPtr[Idx] == NeedlePtr[0]
                && HaystackPtr[Idx + Needle.WriteCur-1] == NeedlePtr[Needle.WriteCur-1]
                && EqualBuffers(Buffer(HaystackPtr + Idx, Needle.WriteCur, 0), Needle))
            {
                return Idx;
            }
        }
        
        if (HaystackPtr[0] == NeedlePtr[0]
            && HaystackPtr[Needle.WriteCur-1] == NeedlePtr[Needle.WriteCur-1]
            && EqualBuffers(Buffer(HaystackPtr, Needle.WriteCur, 0), Needle))
        {
            return 0;
        }
    }
    return INVALID_IDX;
}
internal usz (*_ReverseBufferInBufferIdx)(buffer, buffer) = &_ReverseBufferInBufferIdxSimple;

internal usz
_ReverseBufferInBufferIdxFind(buffer Needle, buffer Haystack)
{
    usz Result = _ReverseBufferInBufferIdx(Needle, Haystack);
    return Result;
}

internal usz
_ReverseBufferInBufferIdxAfter(buffer Needle, buffer Haystack)
{
    usz Idx = _ReverseBufferInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? Idx + Needle.WriteCur : Idx;
    return Result;
}

internal usz
_ReverseBufferInBufferPtrFind(buffer Needle, buffer Haystack)
{
    usz Idx = _ReverseBufferInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? (usz)Haystack.Base + Idx : 0;
    return Result;
}

internal usz
_ReverseBufferInBufferPtrAfter(buffer Needle, buffer Haystack)
{
    usz Idx = _ReverseBufferInBufferIdx(Needle, Haystack);
    usz Result = (Idx != INVALID_IDX) ? (usz)Haystack.Base + Idx + Needle.WriteCur : 0;
    return Result;
}

internal usz
_ReverseBufferInBufferBool(buffer Needle, buffer Haystack)
{
    usz Idx = _ReverseBufferInBufferIdx(Needle, Haystack);
    return Idx != INVALID_IDX;
}

external usz
BufferInBuffer(buffer Needle, buffer Haystack, int Flags)
{
    return Flags & SEARCH_REVERSE ?
    (  Flags & RETURN_BOOL      ? _ReverseBufferInBufferBool(Needle, Haystack)
     : Flags & RETURN_IDX_FIND  ? _ReverseBufferInBufferIdxFind(Needle, Haystack)
     : Flags & RETURN_IDX_AFTER ? _ReverseBufferInBufferIdxAfter(Needle, Haystack)
     : Flags & RETURN_PTR_AFTER ? _ReverseBufferInBufferPtrAfter(Needle, Haystack)
     :                            _ReverseBufferInBufferPtrFind(Needle, Haystack)) :
    (  Flags & RETURN_BOOL      ? _BufferInBufferBool(Needle, Haystack)
     : Flags & RETURN_IDX_FIND  ? _BufferInBufferIdxFind(Needle, Haystack)
     : Flags & RETURN_IDX_AFTER ? _BufferInBufferIdxAfter(Needle, Haystack)
     : Flags & RETURN_PTR_AFTER ? _BufferInBufferPtrAfter(Needle, Haystack)
     :                            _BufferInBufferPtrFind(Needle, Haystack));
}

external usz
DataInBuffer(void* Needle, usz NeedleSize, buffer Haystack, int Flags)
{
    usz Result = BufferInBuffer(Buffer(Needle, NeedleSize, 0), Haystack, Flags);
    return Result;
}

//==================================
// Arena
//==================================

external void*
PushIntoArena(buffer* Arena, usz Size)
{
    void* Result = NULL;
    if (Arena->WriteCur + Size <= Arena->Size)
    {
        Result = (u8*)Arena->Base + Arena->WriteCur;
        Arena->WriteCur += Size;
    }
    return Result;
}

//==================================
// Architecture-dependent code
//==================================

#if defined(TT_X64)

#define CMP_FLAGS _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ORDERED | _SIDD_LEAST_SIGNIFICANT

internal usz
_ByteInBufferIdxAVX2(u8 Needle, buffer Haystack)
{
    usz FoundIdx = INVALID_IDX;
    usz PartialIdx = 0;
    
    if (Haystack.WriteCur > XMM256_SIZE)
    {
        __m256i* Init = (__m256i*)((0-(isz)XMM256_SIZE) & (isz)Haystack.Base);
        __m256i Chunk = _mm256_loadu_si256(Init);
        __m256i Match = _mm256_set1_epi8(Needle);
        
        __m256i Temp = _mm256_cmpeq_epi8(Chunk, Match);
        unsigned long Offset = (unsigned long)((XMM256_LAST_IDX) & (isz)Haystack.Base);
        u32 Mask = _mm256_movemask_epi8(Temp) & (~0u << Offset);
        
        usz RemainingSize = Haystack.WriteCur - (XMM256_SIZE - Offset);
        while (RemainingSize >= XMM256_SIZE
               && Mask == 0)
        {
            Chunk = _mm256_loadu_si256(++Init);
            Temp = _mm256_cmpeq_epi8(Chunk, Match);
            Mask = _mm256_movemask_epi8(Temp);
            RemainingSize -= XMM256_SIZE;
        }
        
        if (Mask != 0)
        {
            Offset = GetFirstBitSet(Mask);
            FoundIdx = (isz)Init - (isz)Haystack.Base + Offset;
        }
        else
        {
            PartialIdx = (isz)++Init - (isz)Haystack.Base;
            Haystack.Base = (u8*)Init;
            Haystack.WriteCur = RemainingSize;
        }
    }
    
    if (FoundIdx == INVALID_IDX)
    {
        for (usz Idx = 0; Idx < Haystack.WriteCur; Idx++)
        {
            if (Haystack.Base[Idx] == Needle)
            {
                FoundIdx = Idx + PartialIdx;
                break;
            }
        }
    }
    
    return FoundIdx;
}

internal usz
_ByteInBufferIdxSSE2(u8 Needle, buffer Haystack)
{
    usz FoundIdx = INVALID_IDX;
    usz PartialIdx = 0;
    
    if (Haystack.WriteCur > XMM128_SIZE)
    {
        __m128i* Init = (__m128i*)((0-(isz)XMM128_SIZE) & (isz)Haystack.Base);
        __m128i Chunk = _mm_load_si128(Init);
        __m128i Match = _mm_set1_epi8(Needle);
        
        __m128i Temp = _mm_cmpeq_epi8(Chunk, Match);
        unsigned long Offset = (u32)((XMM128_SIZE-1) & (isz)Haystack.Base);
        u32 Mask = _mm_movemask_epi8(Temp) & (~0u << Offset);
        
        usz RemainingSize = Haystack.WriteCur - (XMM128_SIZE - Offset);
        while (RemainingSize >= XMM128_SIZE
               && Mask == 0)
        {
            Chunk = _mm_load_si128(++Init);
            Temp = _mm_cmpeq_epi8(Chunk, Match);
            Mask = _mm_movemask_epi8(Temp);
            RemainingSize -= XMM128_SIZE;
        }
        
        if (Mask != 0)
        {
            Offset = GetFirstBitSet(Mask);
            FoundIdx = (isz)Init - (isz)Haystack.Base + Offset;
        }
        else
        {
            PartialIdx = (isz)++Init - (isz)Haystack.Base;
            Haystack.Base = (u8*)Init;
            Haystack.WriteCur = RemainingSize;
        }
    }
    
    if (FoundIdx == INVALID_IDX)
    {
        for (usz Idx = 0; Idx < Haystack.WriteCur; Idx++)
        {
            if (Haystack.Base[Idx] == Needle)
            {
                FoundIdx = Idx + PartialIdx;
                break;
            }
        }
    }
    
    return FoundIdx;
}

internal usz
_BufferInBufferIdxAVX2(buffer Needle, buffer Haystack)
{
    if (Haystack.WriteCur >= Needle.WriteCur)
    {
        __m256i FirstByte = _mm256_set1_epi8(Needle.Base[0]);
        __m256i LastByte  = _mm256_set1_epi8(Needle.Base[Needle.WriteCur-1]);
        
        u8* Ptr;
        for (Ptr = Haystack.Base
             ; Ptr < (Haystack.Base+Haystack.WriteCur-Needle.WriteCur)
             ; Ptr += 32)
        {
            __m256i FirstBlock = _mm256_loadu_si256((__m256i*)&Ptr[0]);
            __m256i LastBlock  = _mm256_loadu_si256((__m256i*)&Ptr[Needle.WriteCur-1]);
            
            __m256i FirstCmp = _mm256_cmpeq_epi8(FirstByte, FirstBlock);
            __m256i LastCmp  = _mm256_cmpeq_epi8(LastByte, LastByte);
            
            u32 Mask = _mm256_movemask_epi8(_mm256_and_si256(FirstCmp, LastCmp));
            while (Mask != 0)
            {
                i32 BitPos = GetFirstBitSet(Mask);
                
                u8* Start = Ptr + BitPos;
                if (memcmp(Start, Needle.Base, Needle.WriteCur) == 0)
                {
                    return (Start - Haystack.Base);
                }
                
                Mask = FlipBit(Mask, BitPos);
            }
        }
        
        while (Ptr < (Haystack.Base+Haystack.WriteCur-Needle.WriteCur))
        {
            if ((Ptr[0] == Needle.Base[0])
                && (Ptr[Needle.WriteCur-1] == Needle.Base[Needle.WriteCur-1]))
            {
                if (memcmp(Ptr, Needle.Base, Needle.WriteCur) == 0)
                {
                    return (Ptr - Haystack.Base);
                }
            }
            Ptr++;
        }
    }
    
    return INVALID_IDX;
}

internal usz
__BufferInBufferIdxAVX2(buffer Needle, buffer Haystack)
{
    if (Haystack.WriteCur >= Needle.WriteCur)
    {
        u8* HaystackPtr = Haystack.Base;
        usz HaystackRemaining = Haystack.WriteCur;
        u8 NeedleEnd = Needle.Base[Needle.WriteCur-1];
        
        if (Needle.WriteCur >= XMM128_SIZE)
        {
            __m128i NeedleChunk = _mm_loadu_si128((__m128i*)Needle.Base);
            while (HaystackRemaining >= XMM128_SIZE)
            {
                __m128i HaystackChunk = _mm_loadu_si128((__m128i*)HaystackPtr);
                int Offset = _mm_cmpistri(NeedleChunk, HaystackChunk, CMP_FLAGS);
                HaystackPtr += Offset;
                HaystackRemaining -= Offset;
                
                if (Offset != 16
                    && HaystackRemaining >= Needle.WriteCur
                    && HaystackPtr[Needle.WriteCur-1] == NeedleEnd
                    && EqualBuffers(Buffer(HaystackPtr, Needle.WriteCur, 0), Needle))
                {
                    return Haystack.WriteCur - HaystackRemaining;
                }
            }
        }
        
        else
        {
            const __m256i FirstChar = _mm256_set1_epi8(Needle.Base[0]);
            while (HaystackRemaining >= XMM256_SIZE
                   && HaystackRemaining >= Needle.WriteCur)
            {
                const __m256i HaystackChunk = _mm256_lddqu_si256((__m256i*)HaystackPtr);
                const __m256i Compare = _mm256_cmpeq_epi8(FirstChar, HaystackChunk);
                
                u32 Mask = _mm256_movemask_epi8(Compare);
                while (Mask != 0)
                {
                    i32 MatchIdx = GetFirstBitSet(Mask);
                    usz TempRemaining = HaystackRemaining - MatchIdx;
                    if (TempRemaining >= Needle.WriteCur
                        && HaystackPtr[MatchIdx + Needle.WriteCur -1] == NeedleEnd
                        && EqualBuffers(Buffer(HaystackPtr + MatchIdx, Needle.WriteCur, 0), Needle))
                    {
                        return Haystack.WriteCur - TempRemaining;
                    }
                    Mask = ClearBit(Mask, MatchIdx);
                }
                HaystackPtr += XMM256_SIZE;
                HaystackRemaining -= XMM256_SIZE;
            }
        }
        
        while (HaystackRemaining >= Needle.WriteCur)
        {
            if (HaystackPtr[0] == Needle.Base[0]
                && HaystackPtr[Needle.WriteCur-1] == NeedleEnd)
            {
                if (EqualBuffers(Buffer(HaystackPtr, Needle.WriteCur, 0), Needle))
                {
                    return (usz)HaystackPtr - (usz)Haystack.Base;
                }
            }
            HaystackPtr++;
            HaystackRemaining--;
        }
    }
    
    return INVALID_IDX;
}

internal usz
_BufferInBufferIdxSSE3(buffer Needle, buffer Haystack)
{
    if (Haystack.WriteCur >= Needle.WriteCur)
    {
        u8* HaystackPtr = Haystack.Base;
        usz HaystackRemaining = Haystack.WriteCur;
        u8 NeedleEnd = Needle.Base[Needle.WriteCur-1];
        
        const __m128i FirstChar = _mm_set1_epi8(Needle.Base[0]);
        while (HaystackRemaining >= XMM128_SIZE
               && HaystackRemaining >= Needle.WriteCur)
        {
            const __m128i HaystackChunk = _mm_lddqu_si128((__m128i*)HaystackPtr);
            const __m128i Compare = _mm_cmpeq_epi8(FirstChar, HaystackChunk);
            
            u32 Mask = _mm_movemask_epi8(Compare);
            while (Mask != 0)
            {
                i32 MatchIdx = GetFirstBitSet(Mask);
                usz TempRemaining = HaystackRemaining - MatchIdx;
                if (TempRemaining >= Needle.WriteCur
                    && HaystackPtr[MatchIdx + Needle.WriteCur -1] == NeedleEnd
                    && EqualBuffers(Buffer(HaystackPtr + MatchIdx, Needle.WriteCur, 0), Needle))
                {
                    return Haystack.WriteCur - TempRemaining;
                }
                Mask = ClearBit(Mask, MatchIdx);
            }
            HaystackPtr += XMM128_SIZE;
            HaystackRemaining -= XMM128_SIZE;
        }
        
        while (HaystackRemaining >= Needle.WriteCur)
        {
            if (HaystackPtr[0] == Needle.Base[0]
                && HaystackPtr[Needle.WriteCur-1] == NeedleEnd)
            {
                if (EqualBuffers(Buffer(HaystackPtr, Needle.WriteCur, 0), Needle))
                {
                    return (usz)HaystackPtr - (usz)Haystack.Base;
                }
            }
            HaystackPtr++;
            HaystackRemaining--;
        }
    }
    
    return INVALID_IDX;
}

// TODO: Implement ReverseByteInBuffer in SIMD.
// TODO: Implement ReverseBufferInBuffer in SIMD.

#endif //TT_X64

external void
InitBuffersArch(void)
{
    LoadCPUArch();
    
#if defined(TT_X64)
    if (CPUIDLeaf1[3] >> 26 & 1) // SSE2
    {
        _ByteInBufferIdx = &_ByteInBufferIdxSSE2;
    }
    if (CPUIDLeaf1[2] >> 0 & 1)  // SSE3
    {
        _BufferInBufferIdx = &_BufferInBufferIdxSSE3;
    }
    if (CPUIDLeaf7a[1] >> 5 & 1) // AVX2
    {
        _ByteInBufferIdx= &_ByteInBufferIdxAVX2;
        _BufferInBufferIdx= &_BufferInBufferIdxAVX2;
    }
#else // OBS: Other platforms.
#endif //TT_X64
}