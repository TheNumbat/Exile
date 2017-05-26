
#pragma once

#include "common.h"

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

logger make_logger(allocator* a);
void destroy_logger(logger* log); // calls logger_stop if needed, call log_end_thread() everywhere first

void logger_init_thread(logger* log, string name); // initializes logging for this thread. call before start in main
void logger_start(logger* log); // begin thread
void logger_stop(logger* log);  // end thread
void logger_init_thread(logger* log, string name, code_context context);
void destroy_logger(logger* log);
i32 logging_thread(void* data_);

logger make_logger(allocator* a) {

	logger ret;

	ret.out = make_vector<log_file>(4, a);
	ret.message_queue = make_queue<log_message>(8, a);
	global_platform_api->platform_create_mutex(&ret.queue_mutex, false);
	global_platform_api->platform_create_mutex(&ret.thread_data_mutex, false);
	global_platform_api->platform_create_semaphore(&ret.logging_semaphore, 0, UINT32_MAX);
	ret.thread_data = make_map<platform_thread_id,log_thread_data>(8, a);
	ret.alloc = a;

	return ret;
}

void logger_start(logger* log) {

	log->thread_param.out 				= &log->out;
	log->thread_param.message_queue 	= &log->message_queue;
	log->thread_param.queue_mutex		= &log->queue_mutex;
	log->thread_param.logging_semaphore = &log->logging_semaphore;
	log->thread_param.running 			= true;

	global_platform_api->platform_create_thread(&log->logging_thread, &logging_thread, &log->thread_param, false);
}

void logger_stop(logger* log) {

	log->thread_param.running = false;

	global_platform_api->platform_signal_semaphore(&log->logging_semaphore, 1);
	global_platform_api->platform_join_thread(&log->logging_thread, -1);
	global_platform_api->platform_destroy_thread(&log->logging_thread);

	log->thread_param.out 				= NULL;
	log->thread_param.message_queue 	= NULL;
	log->thread_param.queue_mutex		= NULL;
	log->thread_param.logging_semaphore = NULL;
}

#define LOGGER_INIT_THREAD(l, n) logger_init_thread(l, n, CONTEXT)
void logger_init_thread(logger* log, string name, code_context context) {

	log_thread_data this_data;

	this_data.context_name = make_stack<string>(8, log->alloc);
	this_data.name = name;
	this_data.start_context = context;

	global_platform_api->platform_aquire_mutex(&log->thread_data_mutex, -1);
	map_insert_if_unique(&log->thread_data, global_platform_api->platform_this_thread_id(), this_data);
	global_platform_api->platform_release_mutex(&log->thread_data_mutex);
}

void destroy_logger(logger* log) {

	if(log->thread_param.running) {
		logger_stop(log);
	}

	destroy_vector(&log->out);
	destroy_queue(&log->message_queue);
	global_platform_api->platform_destroy_mutex(&log->queue_mutex);
	global_platform_api->platform_destroy_mutex(&log->thread_data_mutex);
	global_platform_api->platform_destroy_semaphore(&log->logging_semaphore);
	destroy_map(&log->thread_data);
	log->alloc = NULL;
}

i32 logging_thread(void* data_) {

	log_thread_param* data = (log_thread_param*)data_;	

	return 0;
}