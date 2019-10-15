
#pragma once

#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <new>
#include <atomic>
#include <functional>

#ifdef RUNNING_META
    #define norefl __attribute__((annotate("noreflect")))
#else
    #define norefl
#endif

typedef uint8_t 	u8;
typedef int8_t 		i8;
typedef uint16_t 	u16;
typedef int16_t 	i16;
typedef uint32_t 	u32;
typedef int32_t 	i32;
typedef uint64_t 	u64;
typedef int64_t 	i64;

typedef uintptr_t uptr;
typedef intptr_t  iptr;

typedef float  f32;
typedef double f64;

typedef size_t usize;

#define null nullptr

template <typename F>
struct Defer {
    F f;
    Defer(F f) : f(f) {}
    ~Defer() { f(); }
};

template <typename F>
Defer<F> make_defer(F f) {
    return Defer<F>(f);
}

#define DEFER1(x, y) x##y
#define DEFER2(x, y) DEFER1(x, y)
#define DEFER3(x)    DEFER2(x, __COUNTER__)
#define defer(code)  auto DEFER3(_defer_) = make_defer([&](){code;})
