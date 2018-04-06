
#ifndef RELEASE
	#ifdef CHECKS
		#define BOUNDS_CHECK
		#define BLOCK_OR_EXIT_ON_ERROR
		#define ZERO_ARENA
		#define CHECKED(func, ...) {platform_error err = global_api->func(__VA_ARGS__); if(!err.good) LOG_ERR_F("Error % in %", err.error, #func);}
	#endif
#endif
#ifdef RELEASE
#ifdef PROFILE
	#undef PROFILE
#endif
#endif

#ifndef CHECKED
#define CHECKED(func, ...) global_api->func(__VA_ARGS__);
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

#include <stddef.h>
#include <stdarg.h>
#include <typeinfo>
#include <new>

#include "util/basic_types.h"
#include "math.h"

#include "ds/string.h"
#include "util/context.h"

#include "platform/platform_api.h"
#include "util/fptr.h"

#include "ds/new_alloc.h"
#include "ds/alloc.h"
#include "ds/vector.h"
#include "ds/stack.h"
#include "ds/array.h"
#include "ds/queue.h"
#include "ds/map.h"
#include "ds/buffer.h"
#include "ds/heap.h"

#include "log_html.h"
#include "log.h"
#include "dbg.h"
#include "threads.h"
#include "asset.h"
#include "render_commands.h"
#include "render.h"
#include "events.h"
#include "gui.h"
#include "engine.h"

#include "util/threadstate.h"
#include "util/fscope.h"
#include "util/type_table.h"

// IMPLEMENTATIONS
static platform_api* global_api = null; // global because it just represents a bunch of what should be free functions
static log_manager*  global_log = null; // global to provide printf() like functionality everywhere
static dbg_manager*  global_dbg = null; // global to provide profiling functionality everywhere

#include "util/fptr.cpp"
#include "util/context.cpp"
#include "util/fscope.cpp"
#include "util/threadstate.cpp"
#include "util/type_table.cpp"

#include "log.cpp"
#include "events.cpp"
#include "render_commands.cpp"
#include "render.cpp"
#include "gui.cpp"
#include "dbg.cpp"
#include "asset.cpp"
#include "threads.cpp"
#include "engine.cpp"

#include "ds/new_alloc.cpp"
#include "ds/alloc.cpp"
#include "ds/string.cpp"
#include "ds/vector.cpp"
#include "ds/stack.cpp"
#include "ds/array.cpp"
#include "ds/queue.cpp"
#include "ds/map.cpp"
#include "ds/buffer.cpp"
#include "ds/heap.cpp"
// /IMPLEMENTATIONS
