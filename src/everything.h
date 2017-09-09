
#pragma once

/* debug options

#define BOUNDS_CHECK 			// check access on array_get/vector_get
	
#define BLOCK_ON_ERROR			// __debugbreak on log_level::error
	
#define DO_PROF					// do function enter/exit profiling
	
#define MORE_PROF				// do profiling for functions that are called a _lot_
								// (still excludes vec constructors)

#define ZERO_ARENA				// memset arena allocator store to zero on reset

#define LOG_ALLOCS				// enable logging for every allocation/free/reallocation (including arena - just don't use this it generates way too much)
	
#define CONSTRUCT_DS_ELEMENTS 	// removes need for zero-cleared allocation

#define NO_CONCURRENT_JOBS		// makes queue_job just run the job and wait_job do nothing
*/

#define REAL_RELEASE // turn off everything for a true release build

#ifdef _DEBUG
	#define BOUNDS_CHECK
	#define BLOCK_ON_ERROR
	#define DO_PROF	
	#define CONSTRUCT_DS_ELEMENTS
	// #define MORE_PROF
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

#include "basic_types.h"
#include "math.h"
#include "ds/string.h"

struct code_context {
	string file;
	string function;
	u32 line = 0;
};

#include "platform/platform_api.h"
#include "ds/functions.h"

#include "alloc.h"

#ifdef _DEBUG
#define CHECKED(platform_func, ...) {platform_error err = global_api->platform_func(##__VA_ARGS__); if(!err.good) LOG_ERR_F("Error % in %", err.error, #platform_func);}
#else
#define CHECKED(platform_func, ...) platform_func(##__VA_ARGS__);
#endif

#include "ds/vector.h"
#include "ds/stack.h"
#include "ds/array.h"
#include "ds/queue.h"
#include "ds/map.h"
#include "ds/buffer.h"
#include "ds/heap.h"

#ifdef CONSTRUCT_DS_ELEMENTS
#include <new>
#endif

struct func_ptr_state {

	_FPTR all_ptrs[256];
	u32 num_ptrs = 0;

	platform_dll* this_dll = null;
	
	void reload_all();
};

#include "log_html.h"
#include "log.h"
#include "dbg.h"

#define MAX_CALL_STACK_DEPTH 256
#define MAX_DBG_MSG_CACHE 8192
struct thread_data {
	stack<allocator*> alloc_stack;
	
	string name;
	code_context start_context;

	code_context call_stack[MAX_CALL_STACK_DEPTH] = {};
	u32 call_stack_depth = 0;

	dbg_msg dbg_cache[MAX_DBG_MSG_CACHE] = {};
	u32 dbg_cache_size = 0;
};

static thread_local thread_data this_thread_data;

#define DO(num) for(i32 __i = 0; __i < num; __i++)

#define CONTEXT _make_context(np_string_literal(__FILE__), np_string_literal(__FUNCNAME__), __LINE__)

inline code_context _make_context(string file, string function, i32 line);

#define begin_thread(fmt, a, ...) _begin_thread(fmt, a, CONTEXT, ##__VA_ARGS__);
template<typename... Targs> void _begin_thread(string fmt, allocator* alloc, code_context start, Targs... args);
void end_thread();

#include "ds/threadpool.h"

#include "asset.h"
#include "render.h"
#include "opengl.h"
#include "gui.h"
#include "events.h"

#include "game.h"
static platform_api* global_api = null; // global because it just represents a bunch of what should be free functions
static log_manager*  global_log = null; // global to provide printf() like functionality everywhere
static dbg_manager*  global_dbg = null; // not used yet -- global to provide profiling functionality everywhere

#ifdef DO_PROF
struct func_scope {
	func_scope(code_context context) {
		LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth < MAX_CALL_STACK_DEPTH);
		LOG_DEBUG_ASSERT(this_thread_data.dbg_cache_size < MAX_DBG_MSG_CACHE);

		this_thread_data.call_stack[this_thread_data.call_stack_depth++] = context;

		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.enter_func.func = context;
		// this_thread_data.dbg_cache[this_thread_data.dbg_cache_size++] = m;
		// printf("%+*s\n", this_thread_data.call_stack_depth + context.function.len, context.function.c_str);
	}
	~func_scope() {
		LOG_DEBUG_ASSERT(this_thread_data.dbg_cache_size < MAX_DBG_MSG_CACHE);

		dbg_msg m;
		m.type = dbg_msg_type::exit_func;
		// this_thread_data.dbg_cache[this_thread_data.dbg_cache_size++] = m;

		this_thread_data.call_stack_depth--;
	}
};

struct func_scope_nocs {
	func_scope_nocs(code_context context) {
		LOG_DEBUG_ASSERT(this_thread_data.dbg_cache_size < MAX_DBG_MSG_CACHE);

		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.enter_func.func = context;
		// this_thread_data.dbg_cache[this_thread_data.dbg_cache_size++] = m;
	}
	~func_scope_nocs() {
		LOG_DEBUG_ASSERT(this_thread_data.dbg_cache_size < MAX_DBG_MSG_CACHE);

		dbg_msg m;
		m.type = dbg_msg_type::exit_func;
		// this_thread_data.dbg_cache[this_thread_data.dbg_cache_size++] = m;
	}
};
#define PROF func_scope __f(CONTEXT);
#define PROF_NOCS func_scope_nocs __f(CONTEXT);
#else
#define PROF 
#define PROF_NOCS
#endif

#include "type_table.h"
#include <meta_types.h>

// IMPLEMENTATIONS
#include "ds/functions.cpp"
#include "ds/string.cpp"
#include "math.cpp"

#include "alloc.cpp"
#include "log.cpp"
#include "events.cpp"
#include "opengl.cpp"
#include "render.cpp"
#include "gui.cpp"
#include "dbg.cpp"
#include "asset.cpp"

#include "ds/vector.cpp"
#include "ds/stack.cpp"
#include "ds/array.cpp"
#include "ds/queue.cpp"
#include "ds/map.cpp"
#include "ds/threadpool.cpp"
#include "ds/buffer.cpp"
#include "ds/heap.cpp"

#include "everything.cpp"
// /IMPLEMENTATIONS
