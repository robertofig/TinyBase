#ifndef TINYBASE_MEMORY_H
//=========================================================================
// tinybase-memory.h
//
// Module for working with buffers. A buffer is defined as a memory region
// with a number of bytes written to it and a max size of usable bytes.
//
// Many C standard library functions that work with memory regions assume
// it to be zero-terminated. This module does not require it, allowing it
// to do memory manipulation to specific number of bytes.
//=========================================================================
#define TINYBASE_MEMORY_H

#include "tinybase-types.h"

#if defined(TT_NO_CRL)
external
{
    void* memcpy(void* Dst, const void* Src, size_t SrcSize);
    void* memset(void* Dst, int Value, size_t Count);
}
#else
#include <string.h>
#endif //TT_NO_CRL

external void InitBuffersArch(void);

/* Call this function at the start of the code to use SIMD versions of the functions,
 |  according to runtime checks. If not called, it falls back to generic C versions.
 |--- Return: nothing. */

//=================================
// Creating and editing
//=================================

typedef struct buffer
{
    u8* Base;
    usz WriteCur;
    usz Size;
} buffer;

/* Structure used throughout the library. [.Base] is where the buffer starts, [.WriteCur] is
 |  how much data it currently has, and [.Size] is the full space of the buffer. If [.Size] is
 |  zero, buffer is read-only. */

external buffer Buffer(void* Ptr, usz WriteCur, usz Size);

/* Creates a new buffer struct.
 |--- Return: buffer.*/

external void ClearBuffer(buffer* Buf);

/* Clears the buffer content and sets its [.WriteCur] to zero.
 |--- Return: nothing. */

external void AdvanceBuffer(buffer* Buf, usz NumBytes);

/* Modifies [Buf] to advance its [.Base] by [NumBytes], and shrink [.WriteCur]
   |  and [.Size] by the same amount.
 |--- Return: nothing. */

external bool CopyData(void* Dst, usz DstSize, void* Src, usz SrcSize);

/* Copies data from [Src] to [Dst], so long as [Src] fits entirely in [Dst].
|--- Return: true if successful, false if not. */

external bool AppendDataToBuffer(void* Src, usz SrcSize, buffer* Dst);

/* Copies [SrcSize] bytes from [Src] into [Dst] from its [.WriteCur], so long as it fits
 |  entirely in the remaining space of [Dst].
|--- Return: true if successful, false if not. */

external bool AppendBufferToBuffer(buffer Src, buffer* Dst);

/* Copies data from [Src] into [Dst] from its [.WriteCur], so long as it fits entirely in the
 |  remaining space of [Dst].
|--- Return: true if successful, false if not. */

external bool AppendBufferToBufferNTimes(buffer Src, usz Count, buffer* Dst);

/* Copies data from [Src] into [Dst] from its [.WriteCur], [Count] number of times, so long as
 |  it fits entirely in the remaining space of [Dst].
|--- Return: true if successful, false if not. */

external void ReplaceByteInBuffer(u8 OldByte, u8 NewByte, buffer Buf);

/* Replaces every instance of [OldByte] in [Buf] with [NewByte].
 |--- Return: nothing. */


//=================================
// Query
//=================================

#define INVALID_IDX USZ_MAX

#define RETURN_BOOL      0x1  // Returns 1 if query was successful, 0 if not.
#define RETURN_IDX_FIND  0x2  // Returns offset into first byte of instance found, or INVALID_IDX if none.
#define RETURN_IDX_AFTER 0x4  // Returns offset one byte after first instance found, or INVALID_IDX if none.
#define RETURN_IDX_DIFF  0x8  // Returns offset where two buffers start to differ, or the length if they don't.
#define RETURN_PTR_FIND  0x10 // Returns pointer of first byte of instance found, or NULL if none.
#define RETURN_PTR_AFTER 0x20 // Returns pointer one byte after first instance found, or INVALID_IDX if none.
#define RETURN_PTR_DIFF  0x40 // Returns pointer where two buffers start to differ, or the length if they don't.
#define SEARCH_REVERSE   0x80 // Performs query backwards from the end of buffers.

external usz ByteInBuffer(u8 Needle, buffer Haystack, int Flags);

/* Searches for instance of [Needle] in [Haystack]. Pass either RETURN_BOOL, RETURN_IDX_FIND,
|  RETURN_IDX_AFTER, RETURN_PTR_FIND, or RETURN_PTR_AFTER to [Flags] to determine return type.
|  Return type flag can be OR'd together with SEARCH_REVERSE.
 |--- Return: based on return type flag. */

external usz BufferInBuffer(buffer Needle, buffer Haystack, int Flags);

/* Searches for instance of [Needle] in [Haystack]. Pass either RETURN_BOOL, RETURN_IDX_FIND,
|  RETURN_IDX_AFTER, RETURN_PTR_FIND, or RETURN_PTR_AFTER to [Flags] to determine return type.
|  Return flag can be OR'd together with SEARCH_REVERSE. If a RETURN_(...)_AFTER flag is used,
|  returns one byte after the [Needle] length (e.g. if [Needle] has 35 bytes of length and
|  was found on offset 400, return is at 436).
|--- Return: based on return type flag. */

external usz DataInBuffer(void* Needle, usz NeedleSize, buffer Haystack, int Flags);

/* Same as BufferInBuffer(), but input passed in differently.
|--- Return: based on return type flag. */

external usz CompareBuffers(buffer A, buffer B, usz AmountToCompare, int Flag);

/* Compares two buffers byte by byte for [AmountToCompare] bytes, until they differ, or
|  until the smaller of each buffer's [.WriteCur]. Pass either RETURN_BOOL, RETURN_IDX_DIFF or
|  RETURN_PTR_DIFF to [Flags], returns the point where they differ, or the amount compared if not.
|--- Return: based on return type flag. */

external bool EqualBuffers(buffer A, buffer B);

/* Compares two buffers byte by byte, to see if they are identical in size and content.
 |--- Return: true if successful, false if not. */


//=================================
// Arena
//=================================

#define PushSize(Arena, Size, Type) (Type*)PushIntoArena(Arena, Size)
#define PushStruct(Arena, Type) (Type*)PushIntoArena(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type*)PushIntoArena(Arena, Count * sizeof(Type))

external void* PushIntoArena(buffer* Arena, usz Size);

/* Creates new region in buffer [Arena] of [Size] bytes, advancing its [.WriteCur]
|  accordingly, if new region fits in buffer.
|--- Return: pointer to the beginning of region if successful, NULL if not. */


#if !defined(TT_STATIC_LIKING)
#include "tinybase-memory.c"
#endif

#endif //TINYBASE_MEMORY_H
