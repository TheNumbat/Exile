
#pragma once

/* debug options

#define BOUNDS_CHECK 			// check access on array_get/vector_get
	
#define BLOCK_ON_ERROR			// __debugbreak on log_level::error
	
#define DO_PROF					// do function enter/exit profiling
	
#define MORE_PROF				// do profiling for functions that are called a _lot_
								// (still excludes vec constructors)

#define ZERO_ARENA				// memset arena allocator store to zero on reset
	
#define CONSTRUCT_DS_ELEMENTS 	// removes need for zero-cleared allocation
*/

#define REAL_RELEASE // turn off everything for a true release build

#ifdef _DEBUG
	#define BOUNDS_CHECK
	#define BLOCK_ON_ERROR
	#define DO_PROF
	#define CONSTRUCT_DS_ELEMENTS
#elif defined(REAL_RELEASE)

#else
	#define BLOCK_ON_ERROR
	#define DO_PROF
	#define MORE_PROF
#endif

#ifdef __clang__
#define NOREFLECT __attribute__((annotate("noreflect")))
#else
#define NOREFLECT
#endif

#include "basic_types.h"
#include "math.h"
#include "ds/string.h"

struct code_context {
	string file;
	string function;
	u32 line = 0;
};

#include "alloc.h"
#include "platform_api.h"

#include "ds/vector.h"
#include "ds/stack.h"
#include "ds/array.h"
#include "ds/queue.h"
#include "ds/map.h"

#include "type_table.h"

#ifdef CONSTRUCT_DS_ELEMENTS
#include <new>
#endif

#define MAX_CALL_STACK_DEPTH 256
struct thread_data {
	stack<allocator*> alloc_stack;
	
	string name;
	code_context start_context;

	code_context call_stack[MAX_CALL_STACK_DEPTH];
	u32 call_stack_depth = 0;
};

thread_local thread_data this_thread_data;

#define FOR(num) 			for(i32 __i = 0; __i < num; __i++)
#define FORVEC(v,code) 		{u32 __i = 0; for(auto it = (v).memory; it != (v).memory + (v).size; __i++, it++) {code}}
#define FORVECCAP(v,code) 	{u32 __i = 0; for(auto it = (v).memory; it != (v).memory + (v).capacity; __i++, it++) {code}}
#define FORVEC_R(v,code)	{u32 __i = (v).size; for(auto it = (v).memory + (v).size; it != (v).memory; __i--, it--) {code}}
#define FORMAP(m,code) 		{u32 __i = 0; for(auto it = (m).contents.memory; it != (m).contents.memory + (m).contents.capacity; __i++, it++) if(it->occupied) {code}}
#define FORARR(a,code) 		FORVECCAP(a,code)
#define INC__COUNTER__ 		{u32 i = __COUNTER__; i = 0;}

string np_substring(string str, u32 start, u32 end);
i32    np_string_last_slash(string str);
string np_string_literal(const char* literal);
string np_string_from_c_str(char* c_str);

#define CONTEXT _make_context(np_string_literal(__FILE__), np_string_literal(__func__), __LINE__)

inline code_context _make_context(string file, string function, i32 line);

#define begin_thread(fmt, a, ...) _begin_thread(fmt, a, CONTEXT, __VA_ARGS__);
template<typename... Targs> void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args);
void end_thread();

#include "ds/threadpool.h"

#include "log.h"
#include "asset.h"
#include "render.h"
#include "opengl.h"
#include "gui.h"
#include "events.h"
#include "dbg.h"

#include "game.h"
static game_state* global_state = NULL;

#ifdef DO_PROF
struct func_scope {
	func_scope(code_context context) {
		LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth < MAX_CALL_STACK_DEPTH);
		this_thread_data.call_stack[this_thread_data.call_stack_depth++] = context;
		// printf("%+*s\n", this_thread_data.call_stack_depth + context.function.len, context.function.c_str);
	}
	~func_scope() {
		this_thread_data.call_stack_depth--;
	}
};

struct func_scope_nocs {
	func_scope_nocs(code_context context) {
		
	}
	~func_scope_nocs() {
		
	}
};
#define PROF func_scope __f(CONTEXT);
#define PROF_NOCS func_scope_nocs __f(CONTEXT);
#else
#define PROF 
#define PROF_NOCS
#endif

template<typename T>
struct test {
	T lul[2][2][2];
};
template struct test<string>;

#include <meta_types.h>

// IMPLEMENTATIONS
#include "ds/string.cpp"
#include "math.cpp"

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
// /IMPLEMENTATIONS
