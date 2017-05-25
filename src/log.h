
#pragma once

#include "common.h"

enum log_level {
	log_debug,		// gratuitous info
	log_info,		// relevant info
	log_warn,		// shouldn't happen, debug later
	log_error,		// shouldn't happen, debug now
	log_critical,	// we are about to crash
};

struct log_file {
	platform_file file;
	log_level 	  level;
	bool 		  log_levels_under = true;
};

struct log_message {

};

struct log_thread_param {
	queue<log_message>*	message_queue;
	platform_mutex*		queue_mutex;
	platform_semaphore*	queue_semaphore;
};

struct log {
	vector<log_file> 	out;
	queue<log_message> 	message_queue;
	platform_mutex		queue_mutex;
	platform_semaphore	queue_semaphore;
	platform_thread		logging_thread;
	bool 				flush_on_message = true;

};

void log_add_file(string path, platform_file_open_op mode, log_level level, bool log_levels_under) {

}

void log_add_file(platform_file file, log_level level, bool log_levels_under) {

}

void log_add_file(log_file file) {

}

