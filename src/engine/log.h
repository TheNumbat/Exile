
#pragma once

#include "basic.h"

#include "ds/alloc.h"
#include "ds/string.h"
#include "ds/array.h"
#include "ds/buffer.h"
#include "ds/vector.h"
#include "ds/queue.h"

#include "util/context.h"

struct log_manager;
extern log_manager*  global_log;

// currently, each message enqueue allocates one arena to hold
// a copy of the message, the thread context stack, and the thread name. 
// the arena is freed after the message is output.

enum class log_level : u8 {
	alloc = 0,	// gratuitous allocation info
	ogl,		// opengl notification info
	debug,		// debug info
	info,		// relevant info
	console, 	// console commands
	warn,		// probably shouldn't happen
	error,		// really shouldn't happen
	fatal,		// assert(false), hangs the thread and will exit the program after output
};

enum class log_out_type : u8 {
	plaintext,
	html,
	custom,
};

struct log_message {
	string msg;
	log_level level = log_level::debug;
	
	array<string> context_stack; // snapshot
	string thread_name;

	code_context publisher;
	arena_allocator arena; // joint allocation of msg, data.context_name, data.name

///////////////////////////////////////////////////////////////////////////////

	// allocate strings with current allocator
	string fmt_call_stack();
	string fmt_file_line();
	// will not allocate, returns literal (don't free it)
	string fmt_level();
};

string fmt_msg(log_message* msg, log_out_type type);

struct log_out {
	log_level 	 level = log_level::debug;
	log_out_type type  = log_out_type::plaintext;
	bool 		 flush_on_message = false;
	union {
		buffer<platform_file,4096> file;			// TODO(max): sizeof(buffer) dependent on parameter - don't do this for union?
		struct {
			func_ptr<void, log_message*, void*> write;
			void * param;
		};
	};
	log_out() : file() {};
};

bool operator==(log_out l, log_out r);

struct log_thread_param {
	bool running 							= false;
	vector<log_out>* 	out					= null;
	locking_queue<log_message>*	message_queue	= null;
	platform_semaphore*	logging_semaphore 	= null;
	platform_mutex* output_mut				= null;
	allocator* alloc 						= null;
	arena_allocator* scratch				= null;
};

struct log_manager {
	vector<log_out> 		out;
	locking_queue<log_message> 	message_queue;
	platform_semaphore		logging_semaphore;
	platform_thread			logging_thread;
	platform_mutex 			output_mut;
	log_thread_param 		thread_param;
	allocator* 				alloc = null;
	arena_allocator 		scratch; // reset whenever (on the logging thread) (currently every message)

///////////////////////////////////////////////////////////////////////////////

	static log_manager make(allocator* a); // allocator must have suppress_messages set
	void destroy(); // calls logger_stop if needed, call log_end_thread() everywhere first

	void start(); // begin logging thread - call from one thread
	void stop();  // end logging thread - call from one thread

	void push_context(string context);
	void pop_context();

	void add_file(platform_file file, log_level level, log_out_type type = log_out_type::plaintext, bool flush = false); // call from one thread before starting
	void add_stdout(log_level level, log_out_type type = log_out_type::plaintext); // call from one thread before starting
	void print_header(log_out* out);
	void print_footer(log_out* out);
	void add_custom_output(log_out out);
	void rem_custom_output(log_out out);

	void msg(string msg, log_level level, code_context context);
	template<typename... Targs>
	void msgf(string fmt, log_level level, code_context context, Targs... args);
};

#define LOG_PUSH_CONTEXT(str) global_log->push_context(str); 
#define LOG_POP_CONTEXT() global_log->pop_context();

#define LOG_INFO(m) 	global_log->msg(m, log_level::info,  CONTEXT);
#define LOG_WARN(m) 	global_log->msg(m, log_level::warn,  CONTEXT);
#define LOG_ERR(m) 		global_log->msg(m, log_level::error, CONTEXT);
#define LOG_FATAL(m) 	global_log->msg(m, log_level::fatal, CONTEXT);

#define LOG_INFO_F(fmt, ...) 	global_log->msgf(fmt, log_level::info,  CONTEXT, ##__VA_ARGS__); 
#define LOG_WARN_F(fmt, ...) 	global_log->msgf(fmt, log_level::warn,  CONTEXT, ##__VA_ARGS__); 
#define LOG_ERR_F(fmt, ...) 	global_log->msgf(fmt, log_level::error, CONTEXT, ##__VA_ARGS__); 
#define LOG_FATAL_F(fmt, ...) 	global_log->msgf(fmt, log_level::fatal, CONTEXT, ##__VA_ARGS__); 

#ifdef _MSC_VER
#define LOG_ASSERT(cond) __pragma(warning(push)) \
						 __pragma(warning(disable:4127)) \
						 {if(!(cond)) LOG_FATAL_F("Assertion % failed!"_, #cond##_);} \
						 __pragma(warning(pop))
#elif defined(__GNUC__)
#define LOG_ASSERT(cond) {if(!(cond)) LOG_FATAL_F("Assertion % failed!"_, #cond##_);}
#else
#define LOG_ASSERT(cond)
#endif

#ifndef RELEASE
	#define LOG_DEBUG(m) 			global_log->msg(m,  log_level::debug, CONTEXT); 
	#define LOG_DEBUG_F(fmt, ...) 	global_log->msgf(fmt, log_level::debug, CONTEXT, ##__VA_ARGS__) 
	#define LOG_OGL_F(fmt, ...)		global_log->msgf(fmt, log_level::ogl,   CONTEXT, ##__VA_ARGS__); 
	#ifdef _MSC_VER
	#define LOG_DEBUG_ASSERT(cond) 	do{if(!(cond)) LOG_FATAL_F("Debug assertion % failed!"_, #cond##_);}while(0)
	#elif defined(__GNUC__)
	#define LOG_DEBUG_ASSERT(cond) 	do{if(!(cond)) LOG_FATAL_F("Debug assertion % failed!"_, #cond##_);}while(0)
	#else
	#define LOG_DEBUG_ASSERT(cond)
	#endif
	#define INVALID_PATH 		   	LOG_FATAL("Invalid path taken!"_); 
#else
	#define LOG_DEBUG(msg) 
	#define LOG_DEBUG_F(fmt, ...) 
	#define LOG_DEBUG_ASSERT(cond) 
	#define LOG_OGL_F(fmt, ...) 
	#define INVALID_PATH 
#endif

i32 log_proc(void* data_);
void do_msg(log_thread_param* data, log_message msg);

#include "util/threadstate.h"

#ifndef COMPILING_META_TYPES
#include "ds/vector.inl"
#include "ds/buffer.inl"
#include "ds/stack.inl"
#include "ds/queue.inl"
#include "ds/map.inl"
#include "ds/string.inl"
#include "ds/array.inl"
#include "ds/heap.inl"

template<typename... Targs> 
void log_manager::msgf(string fmt, log_level level, code_context context, Targs... args) { 

	log_message lmsg;

	u32 msg_len = size_stringf(fmt, args...);
	u32 arena_size = msg_len + this_thread_data.name.len + this_thread_data.context_depth * sizeof(string);
	
	arena_allocator arena = MAKE_ARENA("msg"_, arena_size, alloc);

	PUSH_ALLOC(&arena) {

		lmsg.msg = string::makef(msg_len, fmt, args...);

		lmsg.publisher = context;
		lmsg.level = level;

		lmsg.context_stack = array<string>::make_memory(this_thread_data.context_depth, malloc(sizeof(string) * this_thread_data.context_depth));
		lmsg.thread_name = string::make_copy(this_thread_data.name);
		_memcpy(this_thread_data.context_stack, lmsg.context_stack.memory, sizeof(string) * this_thread_data.context_depth);

		lmsg.arena = arena;
		message_queue.push(lmsg);
		
		global_api->signal_semaphore(&logging_semaphore, 1);

#ifdef BLOCK_OR_EXIT_ON_ERROR
		if(level == log_level::error) {

			if(global_api->is_debugging()) {
				global_api->debug_break();
			}
			global_api->join_thread(&logging_thread, -1);
		}
#endif
		if(level == log_level::fatal) {
			if(global_api->is_debugging()) {
				global_api->debug_break();
			}
			global_api->join_thread(&logging_thread, -1);
		}

	} POP_ALLOC();
}
#endif
