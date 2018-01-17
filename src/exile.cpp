
/* debug options

#define BOUNDS_CHECK 			// check access on array_get/vector_get
	
#define BLOCK_OR_EXIT_ON_ERROR	// __debugbreak in debugger or exit() on log_level::error | log_level::fatal always does this
	
#define PROFILE					// do function enter/exit profiling
	
#define MORE_PROF				// do profiling for functions that are called a _lot_
								// (still excludes vec constructors)

#define ZERO_ARENA				// memset arena allocator store to zero on reset

#define NO_CONCURRENT_JOBS		// makes queue_job just run the job and wait_job do nothing
*/

// #define RELEASE // turn off everything for a true release build

#ifndef RELEASE
	#ifdef CHECKS
		#define BOUNDS_CHECK
		#define BLOCK_OR_EXIT_ON_ERROR
		#define ZERO_ARENA
		#define CHECKED(platform_func, ...) {platform_error err = global_api->platform_func(__VA_ARGS__); if(!err.good) LOG_ERR_F("Error % in %", err.error, #platform_func);}
	#endif
#endif
#ifdef RELEASE
#ifdef PROFILE
	#undef PROFILE
#endif
#endif

#ifndef CHECKED
#define CHECKED(platform_func, ...) global_api->platform_func(__VA_ARGS__);
#endif

#ifdef __clang__
#define NOREFLECT __attribute__((annotate("noreflect")))
#define CIRCULAR __attribute__((annotate("circular")))
#else
#define NOREFLECT
#define CIRCULAR
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

// default headers
#include <math.h> 			// TODO(max): remove
#include <xmmintrin.h>
#include <stddef.h>
#include <stdarg.h>
#include <typeinfo>
#include <new>
#ifdef _MSC_VER
#include <intrin.h>
#endif

#include "util/basic_types.h"
#include "math.h"

#include "ds/string.h"
#include "util/context.h"

#include "platform/platform_api.h"
#include "util/fptr.h"

#include "alloc.h"

#include "ds/vector.h"
#include "ds/stack.h"
#include "ds/array.h"
#include "ds/queue.h"
#include "ds/map.h"
#include "ds/buffer.h"
#include "ds/heap.h"
#include "ds/threadpool.h"

#include "log_html.h"
#include "log.h"
#include "dbg.h"

#include "asset.h"
#include "render_commands.h"
#include "render.h"
#include "events.h"
#include "gui.h"
#include "world.h"

#include "util/threadstate.h"
#include "util/fscope.h"

#include "game.h"
#include "util/type_table.h"

// IMPLEMENTATIONS
static platform_api* global_api = null; // global because it just represents a bunch of what should be free functions
static log_manager*  global_log = null; // global to provide printf() like functionality everywhere
static dbg_manager*  global_dbg = null; // global to provide profiling functionality everywhere

#include <meta_types.cpp>

#include "util/fptr.cpp"
#include "util/context.cpp"
#include "util/fscope.cpp"
#include "util/threadstate.cpp"
#include "util/type_table.cpp"

#include "math.cpp"

#include "alloc.cpp"
#include "log.cpp"
#include "events.cpp"
#include "render_commands.cpp"
#include "render.cpp"
#include "gui.cpp"
#include "dbg.cpp"
#include "asset.cpp"
#include "game.cpp"
#include "world.cpp"

#include "ds/string.cpp"
#include "ds/vector.cpp"
#include "ds/stack.cpp"
#include "ds/array.cpp"
#include "ds/queue.cpp"
#include "ds/map.cpp"
#include "ds/threadpool.cpp"
#include "ds/buffer.cpp"
#include "ds/heap.cpp"
// /IMPLEMENTATIONS
