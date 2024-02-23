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
// Platform defines
//==================================

#if defined(_WIN32)
# define TT_WINDOWS
#elif defined(__linux__)
# define TT_LINUX
#else // Reserved for other platforms.
#endif //_WIN32

//==================================
// Compiler defines
//==================================

#if defined(__clang__)
# define TT_CLANG
#elif defined(_MSC_VER)
# define TT_MSVC
#elif defined(__GNUC__)
# define TT_GCC
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
#else // Reserved for other architectures.
#endif //_M_AMD64 || __X86_64__

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

#define internal static
#define global static
#define local static

#if defined(__cplusplus) && (__cplusplus >= 201103)
// Type already defined.
#else
# if defined(TT_GCC) || defined(TT_CLANG)
#  define thread_local __thread
# elif defined(TT_MSVC)
#  define thread_local __declspec( thread )
# else // Reserved for other compilers.
# endif
#endif

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

#define F32_MIN -3.40282346639e+38
#define F32_MAX 3.40282346639e+38
#define F32_PRECISION 6
#define F64_MIN -1.7976931348623157e+308
#define F64_MAX 1.7976931348623157e+308
#define F64_PRECISION 15

#if defined(INFINITY)
# define INF32 INFINITY
# define INF64 INFINITY
#else
union { u32 I; f32 F; } TT_INF32 = { 0x7f800000 };
union { u64 I; f64 F; } TT_INF64 = { 0x7FF0000000000000 };
# define INF32 TT_INF32.F
# define INF64 TT_INF64.F
#endif

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

#define _opt // Does nothing, used for documentation.


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

internal inline i32
GetFirstBitSet(u32 Mask)
{
#if defined(TT_GCC) || defined(TT_CLANG)
    i32 Result = __builtin_ctz(Mask);
#else
    // Source: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
    local const i32 MultiplyDeBruijnBitPosition[32] = {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };
    
    i32 Result = MultiplyDeBruijnBitPosition[((u32)((Mask & -Mask) * 0x077CB531U)) >> 27];
#endif
    return Result;
}

internal inline u32
FlipBit(u32 Number, i32 BitIdx)
{
    local const u32 Patterns[32] = {
        0xFFFFFFFE, 0xFFFFFFFD, 0xFFFFFFFB, 0xFFFFFFF7,
        0xFFFFFFEF, 0xFFFFFFDF, 0xFFFFFFBF, 0xFFFFFF7F,
        0xFFFFFEFF, 0xFFFFFDFF, 0xFFFFFBFF, 0xFFFFF7FF,
        0xFFFFEFFF, 0xFFFFDFFF, 0xFFFFBFFF, 0xFFFF7FFF,
        0xFFFEFFFF, 0xFFFDFFFF, 0xFFFBFFFF, 0xFFF7FFFF,
        0xFFEFFFFF, 0xFFDFFFFF, 0xFFBFFFFF, 0xFF7FFFFF,
        0xFEFFFFFF, 0xFDFFFFFF, 0xFBFFFFFF, 0xF7FFFFFF,
        0xEFFFFFFF, 0xDFFFFFFF, 0xBFFFFFFF, 0x7FFFFFFF
    };
    u32 Result = Number & Patterns[BitIdx];
    return Result;
}

internal inline usz
RoundDownToPow2(i32 Value)
{
#if defined(TT_MSVC)
    i32 TrailingZero = 64-__lzcnt(Value);
#elif defined(TT_GCC)
    i32 TrailingZero = 64-__builtin_clzl(Value);
#else // Reserved for other compiler intrinsics. 
#endif
    return (usz)(1 << (TrailingZero-1));
}

internal inline u32
ClearBit(u32 Value, i32 Bit)
{
    Value &= ~(1UL << Bit);
    return Value;
}

internal inline f64
Pow(f64 Base, f64 Exponent)
{
#if defined(TT_NO_CMATH)
    return 0; // TODO: Implement.
#else
    return pow(Base, Exponent);
#endif
}

internal inline f64
Sin(f64 Angle)
{
#if defined(TT_NO_CMATH)
    return 0; // TODO: Implement.
#else
    return sin(Angle);
#endif
}

internal inline f64
Cos(f64 Angle)
{
#if defined(TT_NO_CMATH)
    return 0; // TODO: Implement.
#else
    return cos(Angle);
#endif
}

internal inline f64
ACos(f64 Angle)
{
#if defined(TT_NO_CMATH)
    return 0; // TODO: Implement.
#else
    return acos(Angle);
#endif
}

internal inline f64
Sqrt(f64 Value)
{
#if defined(TT_NO_CMATH)
    return 0; // TODO: Implement.
#else
    return sqrt(Value);
#endif
}

internal inline usz
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
