
#pragma once

// TODO(max): test flushing every frame instead of every message
// TODO(max): improve allocation scheme
	// currently, each message sent does three allocations
		// copying the message 				- 	I don't think we can avoid this. Likely can optimize with pool allocator
		// 										at the cost of more complexity
		// copying the thread name string 	- for snapshotting 
		// copying the tread context stack	- for snapshotting
	// We can jointly allocate the stack + the name. Do we want to offload the context concatenation work to the publisher?

enum log_level : i8 {
	log_alloc = -1,	// super gratuitous allocation info
	log_debug,		// gratuitous info
	log_info,		// relevant info
	log_warn,		// shouldn't happen, debug later
	log_error,		// shouldn't happen, debug now
	log_fatal,		// basically assert(false), hangs the thread and will exit the program after output
};

struct log_file {
	platform_file file;
	log_level 	  level;
};

struct log_thread_data {
	stack<string> context_name;
	i32 indent_level = 0; // depth of context_name
	string name;
	code_context start_context;
};

struct log_message {
	string msg;
	log_level level;
	log_thread_data data; // snapshot
	code_context publisher;
	bool needs_free = false;
};

struct log_thread_param {
	bool running 							= false;
	vector<log_file>* 	out					= NULL;
	queue<log_message>*	message_queue		= NULL;
	platform_mutex*		queue_mutex			= NULL;
	platform_semaphore*	logging_semaphore 	= NULL;
	allocator* alloc 						= NULL;
	arena_allocator* scratch				= NULL;
};

struct logger {
	vector<log_file> 	out;
	queue<log_message> 	message_queue;
	platform_mutex		queue_mutex, thread_data_mutex;
	platform_semaphore	logging_semaphore;
	platform_thread		logging_thread;
	map<platform_thread_id,log_thread_data> thread_data;
	log_thread_param 	thread_param;
	allocator* alloc 	= NULL;
	arena_allocator 	scratch;
};


logger make_logger(allocator* a); // allocator must have suppress_messages set
void destroy_logger(logger* log); // calls logger_stop if needed, call log_end_thread() everywhere first

#define LOG_INIT_THREAD(n) logger_init_thread(&global_state->log, n, CONTEXT)
void logger_init_thread(logger* log, string name, code_context context);  // initializes logging for this thread. call before start in main
#define LOG_END_THREAD() logger_end_thread(&global_state->log)
void logger_end_thread(logger* log);
void logger_start(logger* log); // begin logging thread - call from one thread
void logger_stop(logger* log);  // end logging thread - call from one thread

#define LOG_PUSH_CONTEXT(str) logger_push_context(&global_state->log, string_literal(str));
void logger_push_context(logger* log, string context);
#define LOG_POP_CONTEXT() logger_pop_context(&global_state->log);
void logger_pop_context(logger* log);

void logger_add_file(logger* log, platform_file file, log_level level); // call from one thread before starting
void logger_print_header(logger* log, log_file file);

// when you log a string unformatted, it makes a copy
// when you log a string formatted, it allocates a new string (to format)
// these are freed after being printed out

#define LOG_DEBUG(msg) 	logger_msg(&global_state->log, string_literal(msg), log_debug, CONTEXT, true);
#define LOG_INFO(msg) 	logger_msg(&global_state->log, string_literal(msg), log_info,  CONTEXT, true);
#define LOG_WARN(msg) 	logger_msg(&global_state->log, string_literal(msg), log_warn,  CONTEXT, true);
#define LOG_ERR(msg) 	logger_msg(&global_state->log, string_literal(msg), log_error, CONTEXT, true);
#define LOG_FATAL(msg) 	logger_msg(&global_state->log, string_literal(msg), log_fatal, CONTEXT, true);
void logger_msg(logger* log, string msg, log_level level, code_context context, bool copy);

#define LOG_DEBUG_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_debug, CONTEXT, __VA_ARGS__);
#define LOG_INFO_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_info,  CONTEXT, __VA_ARGS__);
#define LOG_WARN_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_warn,  CONTEXT, __VA_ARGS__);
#define LOG_ERR_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_error, CONTEXT, __VA_ARGS__);
#define LOG_FATAL_F(fmt, ...) 	logger_msgf(&global_state->log, string_literal(fmt), log_fatal, CONTEXT, __VA_ARGS__);
void logger_msgf(logger* log, string fmt, log_level level, code_context context, ...);

#define LOG_ASSERT(cond) __pragma(warning(push)) \
						 __pragma(warning(disable:4127)) \
						 {if(!(cond)) LOG_FATAL_F("Assertion %s failed!", #cond);} \
						 __pragma(warning(pop))
#ifdef DEBUG
#define LOG_DEBUG_ASSERT(cond) __pragma(warning(push)) \
						 	   __pragma(warning(disable:4127)) \
						 	   {if(!(cond)) LOG_FATAL_F("Debug assertion %s failed!", #cond);} \
						 	   __pragma(warning(pop))
#define INVALID_PATH 		   LOG_FATAL("Invalid path taken!");
#else
#define LOG_DEBUG_ASSERT(cond)
#define INVALID_PATH
#endif

i32 logging_thread(void* data_);
