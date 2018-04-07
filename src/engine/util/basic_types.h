
#pragma once

#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <stddef.h>

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

#define null nullptr
#define DO(num) for(i32 __i = 0; __i < num; __i++)
