#ifndef TINYBASE_TYPES_H
//=========================================================================
// tinybase-types.h
//
// Header with several machine and type defines, as well as helper
// functions, to be used throughout anything built on top TinyBase. 
//=========================================================================
#define TINYBASE_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <float.h>

//==================================
// Platform and compiler defines
//==================================

#if defined(_WIN32)
# define TT_WINDOWS
# define MAX_PATH_SIZE 520 // 260 wchar_t elements.
# define INVALID_FILE USZ_MAX
# define ASYNC_DATA_SIZE 40 // OVERLAPPED struct.
#elif defined(__linux__)
# define TT_LINUX
# define MAX_PATH_SIZE 4096
# define INVALID_FILE USZ_MAX
# define ASYNC_DATA_SIZE 40 // TODO: change.
#else // Reserved for other platforms.
#endif //_WIN32

#if defined(__clang__)
# define TT_CLANG
#elif defined(_MSC_VER)
# define TT_MSVC
#else // Reserved for other compilers.
#endif //__clang__

//==================================
// Architecture defines
//==================================

#if defined(_M_AMD64) || defined(__amd64__)
# define TT_X64
# if defined(TT_WINDOWS)
#  include <intrin.h>
# else
#  include <x86intrin.h>
# endif //TT_WINDOWS
# define XMM128_SIZE 0x10
# define XMM128_LAST_IDX 0xF
# define XMM256_SIZE 0x20
# define XMM256_LAST_IDX 0x1F
static int CPUIDLeaf1[4] = {0};
static int CPUIDLeaf7a[4] = {0};
#else
// Reserved for other architectures.
#endif //_M_AMD64 || __X86_64__

//==================================
// Type defines
//==================================

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef size_t   usz;
typedef intptr_t isz;
typedef int32_t  b32;

#define internal static
#define global static
#define local static

#define Kilobyte(Number) Number * 1024ULL
#define Megabyte(Number) Number * 1024ULL * 1024ULL
#define Gigabyte(Number) Number * 1024ULL * 1024ULL * 1024ULL
#define Terabyte(Number) Number * 1024ULL * 1024ULL * 1024ULL * 1024ULL

#define I8_MIN 0x80
#define I8_MAX 0x7F
#define I8_MAX_DIGITS 3
#define U8_MIN 0x0
#define U8_MAX 0xFF
#define U8_MAX_DIGITS 3
#define I16_MIN 0x8000
#define I16_MAX 0x7FFF
#define I16_MAX_DIGITS 5
#define U16_MIN 0x0
#define U16_MAX 0xFFFF
#define U16_MAX_DIGITS 5
#define I32_MIN 0x80000000
#define I32_MAX 0x7FFFFFFF
#define I32_MAX_DIGITS 10
#define U32_MIN 0x0
#define U32_MAX 0xFFFFFFFF
#define U32_MAX_DIGITS 10
#define I64_MIN 0x8000000000000000
#define I64_MAX 0x7FFFFFFFFFFFFFFF
#define I64_MAX_DIGITS 19
#define U64_MIN 0x0
#define U64_MAX 0xFFFFFFFFFFFFFFFF
#define U64_MAX_DIGITS 20

#if defined(TT_X64) // 64-bits registers.
# define ISZ_MIN I64_MIN
# define ISZ_MAX I64_MAX
# define ISZ_MAX_DIGITS I32_MAX_DIGITS
# define USZ_MIN U64_MIN
# define USZ_MAX U64_MAX
# define USZ_MAX_DIGITS U64_MAX_DIGITS
#else               // 32-bits registers.
# define ISZ_MIN I32_MIN
# define ISZ_MAX I32_MAX
# define ISZ_MAX_DIGITS I32_MAX_DIGITS
# define USZ_MIN U32_MIN
# define USZ_MAX U32_MAX
# define USZ_MAX_DIGITS U32_MAX_DIGITS
#endif

#define _opt

//==================================
// Library imports
//==================================

#if defined(__cplusplus)
# define external extern "C"
#else
# include <stdbool.h>
# define external extern
#endif //__cplusplus

#if !defined(TT_NO_CMATH)
# include <math.h>
#endif

//==================================
// Auxiliary functions
//==================================

#define Abs(A) ((A) < 0 ? -(A) : (A))
#define Clamp01(A) ((A) < 0 ? 0 : (A) > 1 ? 1 : (A))
#define Min(A, B) ((A) <= (B) ? (A) : (B))
#define Max(A, B) ((A) >= (B) ? (A) : (B))

#define Align(Value, Boundary) (Value-1 & ~(Boundary-1)) + Boundary
#define ArrayCount(Arr) sizeof((Arr))/sizeof((Arr)[0])
#define FlipEndian16(I) ((I >> 8)  & 0xFF | (I << 8) & 0xFF00)
#define FlipEndian16x2(I) (FlipEndian16((u16)I) | FlipEndian16((u16)(I >> 16)) << 16)
#define FlipEndian32(I) ((I >> 24) & 0xFF     | (I >> 8)  & 0xFF00      | \
(I << 8)  & 0xFF0000 | (I << 24) & 0xFF000000)
#define FlipEndian64(I) ((I >> 56) & 0xFF             | (I >> 40) & 0xFF00              | \
(I >> 24) & 0xFF0000         | (I >> 8)  & 0xFF000000          | \
(I << 8)  & 0xFF00000000     | (I << 24) & 0xFF0000000000      | \
(I << 40) & 0xFF000000000000 | (I << 56) & 0xFF00000000000000)

static inline i32
GetFirstBitSet(u32 Mask)
{
    local const i32 MultiplyDeBruijnBitPosition[32] = 
    {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };
    
    i32 Result = MultiplyDeBruijnBitPosition[((u32)((Mask & -Mask) * 0x077CB531U)) >> 27];
    return Result;
}

static inline u32
ClearBit(u32 Value, i32 Bit)
{
    Value &= ~(1UL << Bit);
    return Value;
}

static inline f64
Pow(f64 Base, f64 Exponent)
{
#if defined(TT_NO_CMATH)
    return 0; // TODO: Implement.
#else
    return pow(Base, Exponent);
#endif
}

static inline usz
NumberOfDigits(isz Integer)
{
    usz Count = 0;
    do
    {
        Integer /= 10;
        ++Count;
    } while (Integer != 0);
    return(Count);
}

external void
LoadCPUArch(void)
{
#if defined(TT_X64) || defined(TT_X86)
# if defined(TT_MSVC)
    __cpuid(CPUIDLeaf1, 1);
    __cpuidex(CPUIDLeaf7a, 7, 0);
# else // Reserved for other compilers.
# endif //TT_MSVC
#else // Reserved for other architectures.
#endif //TT_X64 || TT_X86
    
#define TT_ARCH_INFO
}

#endif //TINYBASE_TYPES_H
