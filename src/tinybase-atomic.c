//========================================
// Exchange
//========================================

external i16
AtomicExchange16(void* volatile Dst, i16 Value)
{
#if defined(TT_MSVC)
    i16 OldValue = (i16)InterlockedExchange16((SHORT*)Dst, Value);
#elif defined(TT_GCC)
    i16 OldValue = __atomic_exchange_n((i16*)Dst, Value, __ATOMIC_ACQ_REL);
#else // Reserved for other compilers.
#endif
    return OldValue;
}

external i32
AtomicExchange32(void* volatile Dst, i32 Value)
{
#if defined(TT_MSVC)
    i32 OldValue = InterlockedExchange((LONG*)Dst, Value);
#elif defined(TT_GCC)
    i32 OldValue = __atomic_exchange_n((i32*)Dst, Value, __ATOMIC_ACQ_REL);
#else // Reserved for other compilers.
#endif
    return OldValue;
}

external i64
AtomicExchange64(void* volatile Dst, i64 Value)
{
#if defined(TT_MSVC)
    i64 OldValue = InterlockedExchange64((LONG64*)Dst, Value);
#elif defined(TT_GCC)
    i64 OldValue = __atomic_exchange_n((i64*)Dst, Value, __ATOMIC_ACQ_REL);
#else // Reserved for other compilers.
#endif
    return OldValue;
}

external isz
AtomicExchangeIsz(void* volatile Dst, isz Value)
{
#if defined(TT_X64)
    isz OldValue = (isz)AtomicExchange64(Dst, Value);
#else
    isz OldValue = (isz)AtomicExchange32(Dst, Value);
#endif
    return OldValue;
}

external void*
AtomicExchangePtr(void* volatile* Dst, void* Value)
{
#if defined(TT_MSVC)
    void* OldValue = InterlockedExchangePointer(Dst, Value);
#elif defined(TT_GCC)
    void* OldValue = __atomic_exchange_n(Dst, Value, __ATOMIC_ACQ_REL);
#else // Reserved for other compilers.
#endif
    return OldValue;
}


//========================================
// Compate Exchange
//========================================

external bool
AtomicCompareExchange16(void* volatile Dst, i16 Compare, i16 Value)
{
#if defined(TT_MSVC)
#elif defined(TT_GCC)
    bool Result = __atomic_compare_exchange_n((i16*)Dst, &Compare, Value, 0,
                                              __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
#else // Reserved for other compilers.
#endif
    return Result;
}

external bool
AtomicCompareExchange32(void* volatile Dst, i32 Compare, i32 Value)
{
#if defined(TT_MSVC)
    i16 OldValue = InterlockedCompareExchange((SHORT*)Dst, Value, Compare);
#elif defined(TT_GCC)
    bool Result = __atomic_compare_exchange_n((i32*)Dst, &Compare, Value, 0,
                                              __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
#else // Reserved for other compilers.
#endif
    return Result;
}

external bool
AtomicCompareExchange64(void* volatile Dst, i64 Compare, i64 Value)
{
#if defined(TT_MSVC)
    i32 OldValue = InterlockedCompareExchange((LONG*)Dst, Value, Compare);
#elif defined(TT_GCC)
    bool Result = __atomic_compare_exchange_n((i64*)Dst, &Compare, Value, 0,
                                              __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
#else // Reserved for other compilers.
#endif
    return Result;
}

external bool
AtomicCompareExchangeisz(void* volatile Dst, isz Compare, isz Value)
{
#if defined(TT_X64)
    isz Result = (isz)AtomicCompareExchange64(Dst, Compare, Value);
#else
    isz Result = (isz)AtomicCompareExchange32(Dst, Compare, Value);
#endif
    return Result;
}

external bool
AtomicCompareExchangePtr(void* volatile* Dst, void* Compare, void* Value)
{
#if defined(TT_MSVC)
    void* OldValue = InterlockedCompareExchangePointer(Dst, Value, Compare);
#elif defined(TT_GCC)
    bool Result = __atomic_compare_exchange_n(Dst, &Compare, Value, 0,
                                              __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
#else // Reserved for other compilers.
#endif
    return Result;
}


//========================================
// Add and Fetch
//========================================

external i16
AtomicAddFetch16(void* volatile Dst, i16 Value)
{
#if defined(TT_MSVC)
    i16 Result = (i16)InterlockedAdd(Dst, Value);
#elif defined(TT_GCC)
    i16 Result = __atomic_add_fetch((i16*)Dst, Value, __ATOMIC_ACQ_REL);
#else // Reserved for other compilers.
#endif
    return Result;
}

external i32
AtomicAddFetch32(void* volatile Dst, i32 Value)
{
#if defined(TT_MSVC)
    i32 Result = InterlockedAdd(Dst, Value);
#elif defined(TT_GCC)
    i32 Result = __atomic_add_fetch((i32*)Dst, Value, __ATOMIC_ACQ_REL);
#else // Reserved for other compilers.
#endif
    return Result;
}

external i64
AtomicAddFetch64(void* volatile Dst, i64 Value)
{
#if defined(TT_MSVC)
    i64 Result = InterlockedAdd64(Dst, Value);
#elif defined(TT_GCC)
    i64 Result = __atomic_add_fetch((i64*)Dst, Value, __ATOMIC_ACQ_REL);
#else // Reserved for other compilers.
#endif
    return Result;
}

external isz
AtomicAddFetchIsz(void* volatile Dst, isz Value)
{
#if defined(TT_X64)
    isz Result = (isz)AtomicAddFetch64(Dst, Value);
#else
    isz Result = (isz)AtomicAddFetch32(Dst, Value);
#endif
    return Result;
}

external void*
AtomicAddFetchPtr(void* volatile* Dst, isz Value)
{
#if defined(TT_MSVC)
    void* Result = (void*)InterlockedAdd64(Dst, Value);
#elif defined(TT_GCC)
    void* Result = (void*)__atomic_add_fetch((isz*)Dst, Value, __ATOMIC_ACQ_REL);
#else // Reserved for other compilers.
#endif
    return Result;
}
