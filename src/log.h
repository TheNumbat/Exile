
#pragma once

// currently, each message enqueue allocates one arena to hold
// a copy of the message, the thread context stack, and the thread name. 
// the arena is freed after the message is output.

enum class log_level : u8 {
	none = 0,
	alloc,		// super gratuitous allocation info
	ogl,		// opengl info (ignores notifications)
	debug,		// gratuitous info
	info,		// relevant info
	warn,		// shouldn't happen, debug later
	error,		// shouldn't happen, debug now
	fatal,		// basically assert(false), hangs the thread and will exit the program after output
};

enum class log_out_type : u8 {
	plaintext,
	html,
	custom,
};

struct log_message {
	string msg;
	log_level level = log_level::none;
	
	array<code_context> call_stack; // snapshot
	string thread_name;

	code_context publisher;
	arena_allocator arena; // joint allocation of msg, data.context_name, data.name

///////////////////////////////////////////////////////////////////////////////

	// allocate strings with current allocator
	string fmt_time(); // this takes the current time?? TODO(max): real timers
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
		void (*write)(log_message* msg) = null;
	};
	log_out() : file() {}
};

struct log_thread_param {
	bool running 							= false;
	vector<log_out>* 	out					= null;
	con_queue<log_message>*	message_queue	= null;
	platform_semaphore*	logging_semaphore 	= null;
	allocator* alloc 						= null;
	arena_allocator* scratch				= null;
};

struct log_manager {
	vector<log_out> 		out;
	con_queue<log_message> 	message_queue;
	platform_semaphore		logging_semaphore;
	platform_thread			logging_thread;
	log_thread_param 		thread_param;
	allocator* 				alloc = null;
	arena_allocator 		scratch; // reset whenever (on the logging thread) (currently every message)

///////////////////////////////////////////////////////////////////////////////

	static log_manager make(allocator* a); // allocator must have suppress_messages set
	void destroy(); // calls logger_stop if needed, call log_end_thread() everywhere first

	void start(); // begin logging thread - call from one thread
	void stop();  // end logging thread - call from one thread

	void push_context(string context, code_context fake);
	void pop_context();

	void add_file(platform_file file, log_level level, log_out_type type = log_out_type::plaintext, bool flush = false); // call from one thread before starting
	void print_header(log_out* out);
	void print_footer(log_out* out);
	void add_output(log_out out);

	template<typename... Targs>
	void msgf(string fmt, log_level level, code_context context, Targs... args);
	void msg(string msg, log_level level, code_context context);
};

#define LOG_PUSH_CONTEXT(str) global_log->push_context(str, CONTEXT); 
#define LOG_PUSH_CONTEXT_L(str) global_log->push_context(string_literal(str), CONTEXT); 
#define LOG_POP_CONTEXT() global_log->pop_context();

#define LOG_INFO(m) 	global_log->msg(string_literal(m), log_level::info,  CONTEXT);
#define LOG_WARN(m) 	global_log->msg(string_literal(m), log_level::warn,  CONTEXT);
#define LOG_ERR(m) 		global_log->msg(string_literal(m), log_level::error, CONTEXT);
#define LOG_FATAL(m) 	global_log->msg(string_literal(m), log_level::fatal, CONTEXT);

#define LOG_INFO_F(fmt, ...) 	global_log->msgf(string_literal(fmt), log_level::info,  CONTEXT, ##__VA_ARGS__); 
#define LOG_WARN_F(fmt, ...) 	global_log->msgf(string_literal(fmt), log_level::warn,  CONTEXT, ##__VA_ARGS__); 
#define LOG_ERR_F(fmt, ...) 	global_log->msgf(string_literal(fmt), log_level::error, CONTEXT, ##__VA_ARGS__); 
#define LOG_FATAL_F(fmt, ...) 	global_log->msgf(string_literal(fmt), log_level::fatal, CONTEXT, ##__VA_ARGS__); 

#ifdef _MSC_VER
#define LOG_ASSERT(cond) __pragma(warning(push)) \
						 __pragma(warning(disable:4127)) \
						 {if(!(cond)) LOG_FATAL_F("Assertion % failed!", string_literal(#cond));} \
						 __pragma(warning(pop))
#elif defined(__GNUC__)
#define LOG_ASSERT(cond) {if(!(cond)) LOG_FATAL_F("Assertion % failed!", string_literal(#cond));}
#else
#define LOG__ASSERT(cond)
#endif

#ifdef _DEBUG
	#define LOG_DEBUG(m) 			global_log->msg(string_literal(m),  log_level::debug, CONTEXT); 
	#define LOG_DEBUG_F(fmt, ...) 	global_log->msgf(string_literal(fmt), log_level::debug, CONTEXT, ##__VA_ARGS__) 
	#define LOG_OGL_F(fmt, ...)		global_log->msgf(string_literal(fmt), log_level::ogl,   CONTEXT, ##__VA_ARGS__); 
	#ifdef _MSC_VER
	#define LOG_DEBUG_ASSERT(cond) 	__pragma(warning(push)) \
							 	   	__pragma(warning(disable:4127)) \
							 	   	{if(!(cond)) LOG_FATAL_F("Debug assertion % failed!", string_literal(#cond));} \
							 	   	__pragma(warning(pop))
	#elif defined(__GNUC__)
	#define LOG_DEBUG_ASSERT(cond) 	{if(!(cond)) LOG_FATAL_F("Debug assertion % failed!", string_literal(#cond));}
	#else
	#define LOG_DEBUG_ASSERT(cond)
	#endif
	#define INVALID_PATH 		   	LOG_FATAL("Invalid path taken!"); 
#else
	#define LOG_DEBUG(msg) 
	#define LOG_DEBUG_F(fmt, ...) 
	#define LOG_DEBUG_ASSERT(cond) 
	#define LOG_OGL_F(fmt, ...) 
	#define INVALID_PATH 
#endif

i32 log_proc(void* data_);
