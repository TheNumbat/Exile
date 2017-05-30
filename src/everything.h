
#pragma once

#ifdef _DEBUG
#define BOUNDS_CHECK
// #define BREAK_ERROR
#endif

#include "basic_types.h"
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
#include "events.h"
#include "asset.h"
#include "opengl.h"

#include "game.h"
static game_state* global_state = NULL;

#include "str/strings.cpp"

#include "alloc.cpp"
#include "log.cpp"
#include "events.cpp"
#include "asset.cpp"
#include "opengl.cpp"

#include "ds/vector.cpp"
#include "ds/stack.cpp"
#include "ds/array.cpp"
#include "ds/queue.cpp"
#include "ds/map.cpp"
#include "ds/threadpool.cpp"

#include "everything.cpp"
