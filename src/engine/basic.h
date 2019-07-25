
#pragma once

#ifdef CHECK_PLT
#define CHECKED(func, ...) {platform_error err = global_api->func(__VA_ARGS__); if(!err.good) LOG_ERR_F("Error %:\"%\" in %"_, err.error, err.error_message, #func##_);}
#else
#define CHECKED(func, ...) global_api->func(__VA_ARGS__);
#endif

#define EXTERNC extern "C"
#define CALLBACK DLL_EXPORT

#ifdef RUNNING_META
	#define NOREFLECT __attribute__((annotate("noreflect")))
#else
	#define NOREFLECT
#endif

#ifdef _MSC_VER
	#define __FUNCNAME__ __FUNCTION__
	#define DLL_EXPORT EXTERNC __declspec(dllexport)
	#define DLL_IMPORT EXTERNC __declspec(dllimport)
#elif defined(__clang__)
	#define __FUNCNAME__ __func__
	#define DLL_EXPORT EXTERNC __attribute__((visibility("default")))
	#define DLL_IMPORT EXTERNC
#else
	#error Unsupported compiler
#endif

#include <stdint.h>
#include <float.h>
#include <limits.h>
#include <stddef.h>
#include <typeinfo>
#include <new>

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
