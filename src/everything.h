
#pragma once

// be wary of using code reloading with this, it's shared between platform and game

#include <stdint.h>

#ifdef _DEBUG
#define BOUNDS_CHECK
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

#include "str/strings.h"

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

#include "ds/vector.h"
#include "ds/stack.h"
#include "ds/array.h"
#include "ds/queue.h"
#include "ds/map.h"
#include "ds/threadpool.h"

#include "log.h"
#include "game.h"
#include "events.h"

static game_state* global_state = NULL;

#include "str/strings.cpp"

#include "alloc.cpp"
#include "log.cpp"
#include "events.cpp"

#include "ds/vector.cpp"
#include "ds/stack.cpp"
#include "ds/array.cpp"
#include "ds/queue.cpp"
#include "ds/map.cpp"
#include "ds/threadpool.cpp"

#include "everything.cpp"
