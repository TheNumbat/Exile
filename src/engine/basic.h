
#pragma once

#ifndef CHECK_PLT
#define CHECKED(func, ...) {platform_error err = global_api->func(__VA_ARGS__); if(!err.good) LOG_ERR_F("Error %:\"%\" in %"_, err.error, err.error_message, #func##_);}
#else
#define CHECKED(func, ...) global_api->func(__VA_ARGS__);
#endif

#ifdef __clang__
#define NOREFLECT __attribute__((annotate("noreflect")))
#define CIRCULAR __attribute__((annotate("circular")))
#define RENAME(name) __attribute__((annotate(#name)))
#else
#define NOREFLECT
#define CIRCULAR
#define RENAME(name)
#endif

#ifdef _MSC_VER
#define __FUNCNAME__ __FUNCTION__
#define EXPORT extern "C" __declspec(dllexport)
#elif defined(__GNUC__)
#define __FUNCNAME__ __PRETTY_FUNCTION__
#define EXPORT extern "C" __attribute__((dllexport))
#else
#define __FUNCNAME__ __func__
#endif
#define CALLBACK EXPORT

#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <typeinfo>

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
