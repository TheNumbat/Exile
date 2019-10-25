
#pragma once

#include <stdint.h>
#include <float.h>
#include <stddef.h>
#include <limits.h>
#include <thread>

#ifdef _MSC_VER
	#define __func__ __FUNCTION__
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

#include "math.h"
#include "alloc_before.h"
#include "string_before.h"
#include "log_before.h"

#include "reflect.h"
#include "printf_before.h"

#include "3d_math.h"

#include "vec.h"
#include "virt_vec.h"
#include "array.h"
#include "stack.h"
#include "queue.h"
#include "heap.h"

#include "hash.h"
#include "map.h"

#include "log_after.h"
#include "alloc_after.h"
#include "string_after.h"
#include "printf_after.h"
