
#pragma once

#ifdef _DEBUG
#define BOUNDS_CHECK
#define BLOCK_ON_ERROR
// #define ZERO_ARENA
#endif

#include "basic_types.h"
#include "str/strings.h"
#include "platform_api.h"

struct code_context {
	string file;
	string function;
	i32 line = 0;
};

#include "math.h"

#include "ds/vector.h"
#include "ds/stack.h"
#include "ds/array.h"
#include "ds/queue.h"
#include "ds/map.h"
#include "ds/threadpool.h"
#include "alloc.h"

struct thread_data {
	stack<allocator*> alloc_stack;
	stack<code_context> call_stack;
	string name;
	code_context start_context;
};

thread_local thread_data this_thread_data;

#include "log.h"
#include "asset.h"
#include "render.h"
#include "opengl.h"
#include "gui.h"
#include "events.h"
#include "dbg.h"

#include "game.h"

#define FOR(num) for(i32 __i = 0; __i < num; __i++)
#define FORVEC(v,code) {u32 __i = 0; for(auto it = (v).memory; it != (v).memory + (v).size; __i++, it++) {code}}
#define FORVEC_R(v,code) {u32 __i = (v).size; for(auto it = (v).memory + (v).size; it != (v).memory; __i--, it--) {code}}
#define FORMAP(m,code) {u32 __i = 0; for(auto it = (m).contents.memory; it != (m).contents.memory + (m).contents.capacity; __i++, it++) if(it->occupied) {code}}
#define INC__COUNTER__ {u32 i = __COUNTER__; i = 0;}

#ifdef _DEBUG
#define PENTER { \
					stack_push(&this_thread_data.call_stack, _make_context(string_literal(__FILE__), string_literal(__func__), __LINE__)); \
			   } 
#define PEXIT { \
			   		stack_pop(&this_thread_data.call_stack); \
			  }
#define CONTEXT stack_empty(&this_thread_data.call_stack) ? NULL : stack_top(&this_thread_data.call_stack)
#else
#define PENTER
#define PEXIT
#endif

inline code_context _make_context(string file, string function, i32 line);

#define begin_thread(n, a) _begin_thread(n, a, _make_context(string_literal(__FILE__), string_literal(__func__), __LINE__));
void _begin_thread(string name, allocator* alloc, code_context context);
void end_thread();

static game_state* global_state = NULL;

// IMPLEMENTATIONS
#include "str/strings.cpp"

#include "alloc.cpp"
#include "log.cpp"
#include "events.cpp"
#include "asset.cpp"
#include "opengl.cpp"
#include "render.cpp"
#include "gui.cpp"
#include "dbg.cpp"

#include "ds/vector.cpp"
#include "ds/stack.cpp"
#include "ds/array.cpp"
#include "ds/queue.cpp"
#include "ds/map.cpp"
#include "ds/threadpool.cpp"

#include "everything.cpp"
// END IMPLEMENTATIONS
