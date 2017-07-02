
#pragma once

// currently, each message enqueue allocates one 512-byte arena allocation to hold
// a copy of the message, the thread context stack, and the thread name. 
// the arena is freed after the message is output.

enum class log_level : u8 {
	alloc = 0,	// super gratuitous allocation info
	ogl,		// gratuitous opengl info
	debug,		// gratuitous info
	info,		// relevant info
	warn,		// shouldn't happen, debug later
	error,		// shouldn't happen, debug now
	fatal,		// basically assert(false), hangs the thread and will exit the program after output
};

struct log_message {
	string msg;
	log_level level;
	
	array<code_context> call_stack; // snapshot
	string thread_name;

	code_context publisher;
	arena_allocator arena; // joint allocation of msg, data.context_name, data.name
};

struct log_out {
	log_level 	  level;
	bool custom = false;
	union {
		platform_file file;
		void (*write)(log_message* msg) = NULL;
	};
	log_out() : file(), write() {}
};

struct log_thread_param {
	bool running 							= false;
	vector<log_out>* 	out					= NULL;
	queue<log_message>*	message_queue		= NULL;
	platform_mutex*		queue_mutex			= NULL;
	platform_semaphore*	logging_semaphore 	= NULL;
	allocator* alloc 						= NULL;
	arena_allocator* scratch				= NULL;
};

struct log_manager {
	vector<log_out> 	out;
	queue<log_message> 	message_queue;
	platform_mutex		queue_mutex;
	platform_semaphore	logging_semaphore;
	platform_thread		logging_thread;
	log_thread_param 	thread_param;
	allocator* alloc 	= NULL;
	arena_allocator 	scratch; // reset whenever (on the logging thread) (currently every message)
};

log_manager make_logger(allocator* a); // allocator must have suppress_messages set
void destroy_logger(log_manager* log); // calls logger_stop if needed, call log_end_thread() everywhere first

void logger_start(log_manager* log); // begin logging thread - call from one thread
void logger_stop(log_manager* log);  // end logging thread - call from one thread

#define LOG_PUSH_CONTEXT(str) logger_push_context(&global_state->log, str, CONTEXT); 
#define LOG_PUSH_CONTEXT_L(str) logger_push_context(&global_state->log, string_literal(str), CONTEXT); 
void logger_push_context(log_manager* log, string context, code_context fake);
#define LOG_POP_CONTEXT() logger_pop_context(&global_state->log); 
void logger_pop_context(log_manager* log);

void logger_add_file(log_manager* log, platform_file file, log_level level); // call from one thread before starting
void logger_print_header(log_manager* log, log_out out);
void logger_add_output(log_manager* log, log_out out);
void logger_rem_output(log_manager* log, log_out out);

template<typename... Targs>
void logger_msgf(log_manager* log, string fmt, log_level level, code_context context, Targs... args);
void logger_msg(log_manager* log, string msg, log_level level, code_context context);

// allocate strings with current allocator
string log_fmt_msg(log_message* msg);
string log_fmt_msg_time(log_message* msg);
string log_fmt_msg_call_stack(log_message* msg);
string log_fmt_msg_file_line(log_message* msg);
// will not allocate, returns literal (don't free it)
string log_fmt_msg_level(log_message* msg);

#define LOG_INFO(msg) 	logger_msg(&global_state->log, string_literal(msg), log_level::info,  CONTEXT); 
#define LOG_WARN(msg) 	logger_msg(&global_state->log, string_literal(msg), log_level::warn,  CONTEXT); 
#define LOG_ERR(msg) 	logger_msg(&global_state->log, string_literal(msg), log_level::error, CONTEXT); 
#define LOG_FATAL(msg) 	logger_msg(&global_state->log, string_literal(msg), log_level::fatal, CONTEXT); 

#define LOG_INFO_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_level::info,  CONTEXT, __VA_ARGS__); 
#define LOG_WARN_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_level::warn,  CONTEXT, __VA_ARGS__); 
#define LOG_ERR_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_level::error, CONTEXT, __VA_ARGS__); 
#define LOG_FATAL_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_level::fatal, CONTEXT, __VA_ARGS__); 

#define LOG_ASSERT(cond) __pragma(warning(push)) \
						 __pragma(warning(disable:4127)) \
						 {if(!(cond)) LOG_FATAL_F("Assertion % failed!", string_literal(#cond));} \
						 __pragma(warning(pop))

#ifdef _DEBUG
	#define LOG_DEBUG(msg) 			logger_msg(&global_state->log, string_literal(msg),  log_level::debug, CONTEXT); 
	#define LOG_DEBUG_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_level::debug, CONTEXT, __VA_ARGS__) 
	#define LOG_OGL_F(fmt, ...)		logger_msgf(&global_state->log, string_literal(fmt), log_level::ogl,   CONTEXT, __VA_ARGS__); 
	#define LOG_DEBUG_ASSERT(cond) 	__pragma(warning(push)) \
							 	   	__pragma(warning(disable:4127)) \
							 	   	{if(!(cond)) LOG_FATAL_F("Debug assertion % failed!", string_literal(#cond));} \
							 	   	__pragma(warning(pop))
	#define INVALID_PATH 		   	LOG_FATAL("Invalid path taken!"); 
#else
	#define LOG_DEBUG(msg) 
	#define LOG_DEBUG_F(fmt, ...) 
	#define LOG_DEBUG_ASSERT(cond) 
	#define LOG_OGL_F(fmt, ...) 
	#define INVALID_PATH 
#endif

bool operator==(log_out l, log_out r);

i32 logging_thread(void* data_);
