#include "tinybase-memory.h"

#include <stdio.h>

bool Error = false;
#define Test(Callback, ...) \
do { \
if (!Test##Callback(__VA_ARGS__)) { \
Error = true; \
printf(" [%3d] %-40s ERRO.\n", __LINE__, #Callback"()"); } \
} while (0); \


//
// Memory tests
//

bool TestCopyData(void* Dst, usz DstSize, void* Src, usz SrcSize, bool Expected)
{
    bool Result = CopyData(Dst, DstSize, Src, SrcSize);
    if (Result)
    {
        if (DstSize != SrcSize
            || ((u8*)Dst)[DstSize-1] != ((u8*)Src)[SrcSize-1])
        {
            Result = false;
        }
    }
    return Result == Expected;
}

bool TestClearBuffer(buffer* Buf)
{
    ClearBuffer(Buf);
    return Buf->Base[0] == 0 && Buf->Base[Buf->Size-1] == 0 && Buf->WriteCur == 0;
}

bool TestAppendDataToBuffer(void* Src, usz SrcSize, buffer* Dst, buffer Expected1, bool Expected2)
{
    bool Result = AppendDataToBuffer(Src, SrcSize, Dst);
    if (Result)
    {
        if (Dst->WriteCur != Expected1.WriteCur
            || Dst->Base[Dst->WriteCur-1] != Expected1.Base[Expected1.WriteCur-1])
        {
            Result = false;
        }
    }
    return Result == Expected2;
}

bool TestAppendBufferToBuffer(buffer Src, buffer* Dst, buffer Expected1, bool Expected2)
{
    bool Result = AppendBufferToBuffer(Src, Dst);
    if (Result)
    {
        if (Dst->WriteCur != Expected1.WriteCur
            || Dst->Base[Dst->WriteCur-1] != Expected1.Base[Expected1.WriteCur-1])
        {
            Result = false;
        }
    }
    return Result == Expected2;
}

bool Test_ByteInBufferIdxSimple(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == _ByteInBufferIdxSimple(Needle, Haystack);
}

bool Test_ByteInBufferIdxAVX2(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == _ByteInBufferIdxAVX2(Needle, Haystack);
}

bool Test_ByteInBufferIdxSSE2(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == _ByteInBufferIdxSSE2(Needle, Haystack);
}

bool TestByteInBufferPtrFind(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_PTR_FIND);
}

bool TestByteInBufferPtrAfter(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_PTR_AFTER);
}

bool TestByteInBufferIdxFind(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_IDX_FIND);
}

bool TestByteInBufferIdxAfter(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_IDX_AFTER);
}

bool TestByteInBufferBool(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_BOOL);
}

bool Test_ReverseByteInBufferIdxSimple(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == _ReverseByteInBufferIdxSimple(Needle, Haystack);
}

bool TestReverseByteInBufferPtrFind(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_PTR_FIND | SEARCH_REVERSE);
}

bool TestReverseByteInBufferPtrAfter(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_PTR_AFTER | SEARCH_REVERSE);
}

bool TestReverseByteInBufferIdxFind(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_IDX_FIND | SEARCH_REVERSE);
}

bool TestReverseByteInBufferIdxAfter(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_IDX_AFTER | SEARCH_REVERSE);
}

bool TestReverseByteInBufferBool(u8 Needle, buffer Haystack, usz Expected)
{
    return Expected == ByteInBuffer(Needle, Haystack, RETURN_BOOL | SEARCH_REVERSE);
}

bool Test_BufferInBufferIdxSimple(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == _BufferInBufferIdxSimple(Needle, Haystack);
}

bool Test_BufferInBufferIdxAVX2(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == _BufferInBufferIdxAVX2(Needle, Haystack);
}

bool Test_BufferInBufferIdxSSE3(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == _BufferInBufferIdxSSE3(Needle, Haystack);
}

bool TestBufferInBufferPtrFind(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_PTR_FIND);
}

bool TestBufferInBufferPtrAfter(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_PTR_AFTER);
}

bool TestBufferInBufferIdxFind(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_IDX_FIND);
}

bool TestBufferInBufferIdxAfter(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_IDX_AFTER);
}

bool TestBufferInBufferBool(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_BOOL);
}

bool Test_ReverseBufferInBufferIdxSimple(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == _ReverseBufferInBufferIdxSimple(Needle, Haystack);
}

bool TestReverseBufferInBufferPtrFind(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_PTR_FIND | SEARCH_REVERSE);
}

bool TestReverseBufferInBufferPtrAfter(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_PTR_AFTER | SEARCH_REVERSE);
}

bool TestReverseBufferInBufferIdxFind(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_IDX_FIND | SEARCH_REVERSE);
}

bool TestReverseBufferInBufferIdxAfter(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_IDX_AFTER | SEARCH_REVERSE);
}

bool TestReverseBufferInBufferBool(buffer Needle, buffer Haystack, usz Expected)
{
    return Expected == BufferInBuffer(Needle, Haystack, RETURN_BOOL | SEARCH_REVERSE);
}

bool Test_ComparePtrSimple(void* A, void* B, usz AmountToCompare, usz Expected)
{
    return Expected == _ComparePtrSimple(A, B, AmountToCompare);
}

bool TestComparePtr(buffer A, buffer B, usz AmountToCompare, usz Expected)
{
    return Expected == CompareBuffers(A, B, AmountToCompare, RETURN_PTR_DIFF);
}

bool TestCompareIdx(buffer A, buffer B, usz AmountToCompare, usz Expected)
{
    return Expected == CompareBuffers(A, B, AmountToCompare, RETURN_IDX_DIFF);
}

bool TestCompareBool(buffer A, buffer B, usz AmountToCompare, usz Expected)
{
    return Expected == CompareBuffers(A, B, AmountToCompare, RETURN_BOOL);
}

bool TestEquals(buffer A, buffer B, b32 Expected)
{
    return Expected == EqualBuffers(A, B);
}


//
// Test program
//

int main()
{
    bool HasAVX2 = CPUIDLeaf7a[1] >> 5 & 1;
    bool HasSSE3 = CPUIDLeaf1[2] >>  0 & 1;
    bool HasSSE2 = CPUIDLeaf1[3] >> 26 & 1;
    
    char Buffer1[10] = {0};
    buffer B1 = Buffer(Buffer1, 0, sizeof(Buffer1));
    char Buffer2[10] = "Lorem";
    buffer B2 = Buffer(Buffer2, 5, sizeof(Buffer2));
    char Buffer3[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin nibh purus, iaculis eu interdum vel, suscipit euismod massa. Ut nec feugiat quam, quis mattis sapien. Nulla eget dui et lorem pharetra iaculis. Suspendisse sed dolor ante. Sed eu quam nulla. Donec pretium dapibus nulla a sodales. Nunc aliquet, nibh commodo gravida ultricies, mauris dui aliquet diam, sit amet tristique dolor ligula quis massa. Maecenas ut mauris non risus pretium facilisis nec in enim. Suspendisse et dui id velit rutrum aliquet ut eu sapien. Maecenas elementum, quam eu zuctor tempor, arcu nisi auctor lectus, vitae hendrerit ipsum mauris interdum erat.";
    buffer B3 = Buffer(Buffer3, sizeof(Buffer3)-1, sizeof(Buffer3));
    buffer B4 = Buffer("ipsum", 5, 0);
    buffer B5 = Buffer("ipsun", 5, 0);
    
    Test(CopyData, B1.Base, B1.Size, "Lorem ipsu", 10, true);
    Test(ClearBuffer, &B1);
    Test(CopyData, B1.Base, B1.Size, "Lorem ipsum", 11, false);
    Test(AppendDataToBuffer, " ipsu", 5, &B2, Buffer("Lorem ipsu", 10, 0), true);
    Test(AppendDataToBuffer, "m", 1, &B2, Buffer("Lorem ipsum", 11, 0), false);
    Test(AppendBufferToBuffer, B2, &B1, Buffer("Lorem ipsu", 10, 0), true);
    Test(AppendBufferToBuffer, Buffer("m", 1, 0), &B1, Buffer("Lorem ipsum", 11, 0), false);
    
    if (HasAVX2) Test(_ByteInBufferIdxAVX2, 'z', B3, 553);
    if (HasSSE2) Test(_ByteInBufferIdxSSE2, 'z', B3, 553);
    
    Test(ByteInBufferPtrFind, 'm', B3, (usz)&Buffer3[4]);
    Test(ByteInBufferPtrAfter, 'm', B3, (usz)&Buffer3[5]);
    Test(ByteInBufferIdxFind, 'm', B3, 4);
    Test(ByteInBufferIdxAfter, 'm', B3, 5);
    Test(ByteInBufferBool, 'm', B3, true);
    Test(ByteInBufferPtrFind, '%', B3, 0);
    Test(ByteInBufferPtrAfter, '%', B3, 0);
    Test(ByteInBufferIdxFind, '%', B3, INVALID_IDX);
    Test(ByteInBufferIdxAfter, '%', B3, INVALID_IDX);
    Test(ByteInBufferBool, '%', B3, false);
    
    Test(ReverseByteInBufferPtrFind, 'm', B3, (usz)&Buffer3[629]);
    Test(ReverseByteInBufferPtrAfter, 'm', B3, (usz)&Buffer3[630]);
    Test(ReverseByteInBufferIdxFind, 'm', B3, 629);
    Test(ReverseByteInBufferIdxAfter, 'm', B3, 630);
    Test(ReverseByteInBufferBool, 'm', B3, true);
    Test(ReverseByteInBufferPtrFind, '%', B3, 0);
    Test(ReverseByteInBufferPtrAfter, '%', B3, 0);
    Test(ReverseByteInBufferIdxFind, '%', B3, INVALID_IDX);
    Test(ReverseByteInBufferIdxAfter, '%', B3, INVALID_IDX);
    Test(ReverseByteInBufferBool, '%', B3, false);
    
    if (HasAVX2) Test(_BufferInBufferIdxAVX2, Buffer("zuctor tempor, arcu nisi", 24, 0), B3, 553);
    if (HasSSE3) Test(_BufferInBufferIdxSSE3, Buffer("zuctor tempor, arcu nisi", 24, 0), B3, 553);
    
    Test(BufferInBufferPtrFind, B4, B3, (usz)&Buffer3[6]);
    Test(BufferInBufferPtrAfter, B4, B3, (usz)&Buffer3[11]);
    Test(BufferInBufferIdxFind, B4, B3, 6);
    Test(BufferInBufferIdxAfter, B4, B3, 11);
    Test(BufferInBufferBool, B4, B3, true);
    Test(BufferInBufferPtrFind, B5, B3, 0);
    Test(BufferInBufferPtrAfter, B5, B3, 0);
    Test(BufferInBufferIdxFind, B5, B3, INVALID_IDX);
    Test(BufferInBufferIdxAfter, B5, B3, INVALID_IDX);
    Test(BufferInBufferBool, B5, B3, false);
    
    Test(ReverseBufferInBufferPtrFind, B4, B3, (usz)&Buffer3[609]);
    Test(ReverseBufferInBufferPtrAfter, B4, B3, (usz)&Buffer3[614]);
    Test(ReverseBufferInBufferIdxFind, B4, B3, 609);
    Test(ReverseBufferInBufferIdxAfter, B4, B3, 614);
    Test(ReverseBufferInBufferBool, B4, B3, true);
    Test(ReverseBufferInBufferPtrFind, B5, B3, 0);
    Test(ReverseBufferInBufferPtrAfter, B5, B3, 0);
    Test(ReverseBufferInBufferIdxFind, B5, B3, INVALID_IDX);
    Test(ReverseBufferInBufferIdxAfter, B5, B3, INVALID_IDX);
    Test(ReverseBufferInBufferBool, B5, B3, false);
    
    Test(ComparePtr, B4, B5, 5, (usz)(((u8*)B4.Base) + 4));
    Test(CompareIdx, B4, B5, 5, 4);
    Test(Equals, B4, B4, true);
    Test(ComparePtr, B4, B2, 5, (usz)(((u8*)B4.Base)));
    Test(CompareIdx, B4, B2, 5, 0);
    Test(Equals, B4, B5, false);
    
    if (!Error) printf("All tests passed!\n");
    return 0;
}