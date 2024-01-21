#ifndef TINYBASE_ATOMIC_H
//==========================================================================
// tinybase-atomic.h
//
// Module to generalise atomic operations, which are compiler-specific.
// These operations are meant for multithreaded code, and are guaranteed
// to run in Aquire-Release semantics.
//==========================================================================
#define TINYBASE_ATOMIC_H


//========================================
// Exchange
//========================================

external i16 AtomicExchange16(void* volatile Dst, i16 Value);

/* Saves 16-bit [Value] into [Dst] in a thread-safe manner, and returns previous value.
|--- Result: 16-bit value at [Dst] before function call. */

external i32 AtomicExchange32(void* volatile Dst, i32 Value);

/* Saves 32-bit [Value] into [Dst] in a thread-safe manner, and returns previous value.
|--- Result: 32-bit value at [Dst] before function call. */

external i64 AtomicExchange64(void* volatile Dst, i64 Value);

/* Saves 64-bit [Value] into [Dst] in a thread-safe manner, and returns previous value.
|--- Result: 64-bit value at [Dst] before function call. */

external isz AtomicExchangeIsz(void* volatile Dst, isz Value);

/* Saves 32/64-bit [Value] into [Dst] in a thread-safe manner, and returns previous value.
|--- Result: 32/64-bit value at [Dst] before function call. */

external void* AtomicExchangePtr(void* volatile* Dst, void* Value);

/* Saves pointer [Value] into [Dst] in a thread-safe manner, and returns previous value.
|--- Result: pointer address at [Dst] before function call. */


//========================================
// Compate Exchange
//========================================

external bool AtomicCompareExchange16(void* volatile Dst, i16 Compare, i16 Value);

/* Saves 16-bit [Value] into [Dst] in a thread-safe manner, if the values of [Dst] and
|  [Compare] are the same. If they are different, doesn't do anything.
|--- Return: true if successful, false if not. */

external bool AtomicCompareExchange32(void* volatile Dst, i32 Compare, i32 Value);

/* Saves 32-bit [Value] into [Dst] in a thread-safe manner, if the values of [Dst] and
|  [Compare] are the same. If they are different, doesn't do anything.
|--- Return: true if successful, false if not. */

external bool AtomicCompareExchange64(void* volatile Dst, i64 Compare, i64 Value);

/* Saves 64-bit [Value] into [Dst] in a thread-safe manner, if the values of [Dst] and
|  [Compare] are the same. If they are different, doesn't do anything.
|--- Return: true if successful, false if not. */

external bool AtomicCompareExchangeIsz(void* volatile Dst, isz Compare, isz Value);

/* Saves 32/64-bit [Value] into [Dst] in a thread-safe manner, if the values of [Dst] and
|  [Compare] are the same. If they are different, doesn't do anything.
|--- Return: true if successful, false if not. */

external bool AtomicCompareExchangePtr(void* volatile* Dst, void* Compare, void* Value);

/* Saves pointer [Value] into [Dst] in a thread-safe manner, if the values of [Dst] and
|  [Compare] are the same. If they are different, doesn't do anything.
|--- Return: true if successful, false if not. */


//========================================
// Add and Fetch
//========================================

external i16 AtomicAddFetch16(void* volatile Dst, i16 Value);

/* Adds 16-bit [Value] to the content of [Dst] in a thread-safe manner, and returns
 |  current value.
|--- Return: 16-bit value at [Dst] after function call. */

external i32 AtomicAddFetch32(void* volatile Dst, i32 Value);

/* Adds 32-bit [Value] to the content of [Dst] in a thread-safe manner, and returns
 |  current value.
|--- Return: 32-bit value at [Dst] after function call. */

external i64 AtomicAddFetch64(void* volatile Dst, i64 Value);

/* Adds 64-bit [Value] to the content of [Dst] in a thread-safe manner, and returns
 |  current value.
|--- Return: 64-bit value at [Dst] after function call. */

external isz AtomicAddFetchIsz(void* volatile Dst, isz Value);

/* Adds 32/64-bit [Value] to the content of [Dst] in a thread-safe manner, and returns
 |  current value.
|--- Return: 64-bit value at [Dst] after function call. */

external void* AtomicAddFetchPtr(void* volatile* Dst, isz Value);

/* Adds 64-bit [Value] to the content of [Dst] in a thread-safe manner, and returns
 |  current value.
|--- Return: pointer address at [Dst] after function call. */


#if !defined(TT_STATIC_LINKING)
#include "tinybase-atomic.c"
#endif //TT_STATIC_LINKING

#endif //TINYBASE_ATOMIC_H
