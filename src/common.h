
#pragma once

// be wary of using code reloading with this, it's shared between platform and game

#include <stdint.h>

#define DEBUG

#ifdef DEBUG
	#include <assert.h>
	#define INVALID_PATH assert(false);
	#define BOUNDS_CHECK
#else
	#define assert(t) 
	#define INVALID_PATH
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

#include "strings.h"
#include "context_strings.h"

#include "platform_api.h"

struct code_context {
	string file;
	string function;
	i32 line = 0;
};

inline code_context make_context(string file, string function, i32 line);

#define CONTEXT make_context(string_literal(__FILE__), string_literal(__func__), __LINE__)

#include "alloc.h"
#include "math.h"

#include "vector.h"
#include "stack.h"
#include "array.h"
#include "queue.h"
#include "map.h"
#include "threadpool.h"

#include "log.h"
#include "game.h"

static game_state* global_state = NULL;

#include "strings.cpp"
#include "context_strings.cpp"

#include "alloc.cpp"
#include "log.cpp"

#include "vector.cpp"
#include "stack.cpp"
#include "array.cpp"
#include "queue.cpp"
#include "map.cpp"
#include "threadpool.cpp"
#include "common.cpp"
