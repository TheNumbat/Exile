
#pragma once

// be wary of using code reloading with this, it's shared between platform and game

#include <stdint.h>

#ifdef DEBUG
#define ASSERTS
#define PROFILING
#define LOGGING
#endif

#ifdef ASSERTS
#include <assert.h>
#else
#define assert(t) 
#endif

typedef uint8_t 	u8;
typedef int8_t 		i8;
typedef uint16_t 	u16;
typedef int16_t 	i16;
typedef uint32_t 	u32;
typedef int32_t 	i32;
typedef uint64_t 	u64;
typedef int64_t 	i64;

typedef float  f32;
typedef double f64;

#include "platform_api.h"
#include "strings.h"

struct code_context {
	string file;
	string function;
	i32 line = 0;
};

inline code_context make_context(string file, string function, i32 line) {

	code_context ret;
	ret.file = file;
	ret.function = function;
	ret.line = line;

	return ret;
}

#define CONTEXT make_context(string_literal(__FILE__), string_literal(__func__), __LINE__)

#include "alloc.h"
#include "vector.h"
#include "stack.h"
