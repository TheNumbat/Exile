
#pragma once

// TODO(max): test flushing every frame instead of every message

enum log_level : u8 {
	log_debug = 0,	// gratuitous info
	log_info,		// relevant info
	log_warn,		// shouldn't happen, debug later
	log_error,		// shouldn't happen, debug now
	log_fatal,		// we are about to crash
};

struct log_file {
	platform_file file;
	log_level 	  level;
};

struct log_thread_data {
	stack<string> context_name;
	i32 indent_level = 0;
	string name;
	code_context start_context;
};

struct log_message {
	string msg;
	log_thread_data data; // snapshot
	code_context publisher;
};

struct log_thread_param {
	bool running 							= false;
	vector<log_file>* 	out					= NULL;
	queue<log_message>*	message_queue		= NULL;
	platform_mutex*		queue_mutex			= NULL;
	platform_semaphore*	logging_semaphore 	= NULL;
};

struct logger {
	vector<log_file> 	out;
	queue<log_message> 	message_queue;
	platform_mutex		queue_mutex, thread_data_mutex;
	platform_semaphore	logging_semaphore;
	platform_thread		logging_thread;
	map<platform_thread_id,log_thread_data> thread_data;
	log_thread_param 	thread_param;
	allocator* alloc = NULL;
};

#define LOGGER_INIT_THREAD(l, n) logger_init_thread(l, n, CONTEXT)

logger make_logger(allocator* a);
void destroy_logger(logger* log); // calls logger_stop if needed, call log_end_thread() everywhere first

void logger_init_thread(logger* log, string name); // initializes logging for this thread. call before start in main
void logger_start(logger* log); // begin thread
void logger_stop(logger* log);  // end thread
void logger_init_thread(logger* log, string name, code_context context);
void destroy_logger(logger* log);
i32 logging_thread(void* data_);
