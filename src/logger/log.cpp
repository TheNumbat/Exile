
#include "log.hpp"
#include "../common.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>

logger glog(EMIT_LEVEL);

namespace tbb {
namespace interface5 {
template<>
inline size_t tbb_hasher<std::thread::id> (const std::thread::id& t) {
    return static_cast<size_t>( std::hash<std::thread::id>()(t) );
}
}}

inline const char* LEVEL_STR(message_level lvl) {
	switch(lvl) {
		case message_info: return "INFO";
		case message_warn: return "WARN";
		case message_error: return "ERROR";
		case message_fatal: return "FATAL";
		default: INVALID_CODE_PATH; return nullptr;
	}
}
	
message::message() {
	indent_level = 0;
	lvl = message_fatal;
	context = "NONE";
	msg = "";
	id = std::this_thread::get_id();
}

message::message(int indent, std::thread::id i, message_level level, std::string c, std::string m) { 
	indent_level = indent;
	lvl = level;
	context = c;
	msg = m;
	id = i;
}

logger::logger(int emit) 
	: message_q(alloc<message>("LOG/MESSAGE_QUEUE")),
	  out_streams(alloc<std::ostream*>("LOG/OSTREAM")),
	  thread_indent_levels(alloc_pair_id_int("LOG/TTHREAD_INDENTS")),
	  thread_names(alloc_pair_id_str("LOG/THREAD_NAMES")),
	  thread_current_context(alloc_pair_id_str_stack("LOG/THREAD_CONTEXTS"))
{
	end_thread = false;
	emit_level = emit;
#ifdef LOG_FILE
	filestream.open("log.txt");
	out_streams.push_back(&filestream);
#endif
#ifdef LOG_CONSOLE
	out_streams.push_back(&std::cout);
#endif
	thread = std::move(std::thread(&logger::logging_thread, this));
}

logger::~logger() {
	end_thread = true;
	var.notify_all();
	thread.join();
}

void logger::begin_on_thread(std::string name) {
	std::thread::id id = std::this_thread::get_id();
	thread_indent_levels.insert({id, 0});
	thread_current_context.insert({id, std::stack<std::string>()});
	thread_names.insert({id, name});
}

void logger::pushSec() {
	std::thread::id id = std::this_thread::get_id();
	id_int_map::accessor lacc;
	bool found = thread_indent_levels.find(lacc, id);
	assert(found);
	lacc->second++;
}

void logger::popSec() {
	std::thread::id id = std::this_thread::get_id();
	id_int_map::accessor lacc;
	bool found = thread_indent_levels.find(lacc, id);
	assert(found);
	assert(lacc->second > 0);
	lacc->second--;
}

void logger::pushContext(std::string context) {
	std::thread::id id = std::this_thread::get_id();
	id_stack_str_map::accessor cacc;
	bool found = thread_current_context.find(cacc, id);
	assert(found);
	cacc->second.push(context);
}

void logger::popContext() {
	std::thread::id id = std::this_thread::get_id();
	id_stack_str_map::accessor cacc;
	bool found = thread_current_context.find(cacc, id);
	assert(found);
	assert(cacc->second.size() > 0);
	cacc->second.pop();
}

void logger::info(std::string msg) {
	if(emit_level >= 3) {
		std::thread::id id = std::this_thread::get_id();
		id_int_map::accessor lacc;
		id_stack_str_map::accessor cacc;
		bool found = thread_indent_levels.find(lacc, id);
		assert(found);
		found = thread_current_context.find(cacc, id);
		assert(found);
		assert(cacc->second.size() > 0);
		message_q.push(message(lacc->second, id, message_info, cacc->second.top(), msg));
		var.notify_all();
	}
}

void logger::warn(std::string msg) {
	if(emit_level >= 2) {
		std::thread::id id = std::this_thread::get_id();
		id_int_map::accessor lacc;
		id_stack_str_map::accessor cacc;
		bool found = thread_indent_levels.find(lacc, id);
		assert(found);
		found = thread_current_context.find(cacc, id);
		assert(found);
		assert(cacc->second.size() > 0);
		message_q.push(message(lacc->second, id, message_warn, cacc->second.top(), msg));
		var.notify_all();
	}
}

void logger::error(std::string msg) {
	if(emit_level >= 1) {
		std::thread::id id = std::this_thread::get_id();
		id_int_map::accessor lacc;
		id_stack_str_map::accessor cacc;
		bool found = thread_indent_levels.find(lacc, id);
		assert(found);
		found = thread_current_context.find(cacc, id);
		assert(found);
		assert(cacc->second.size() > 0);
		message_q.push(message(lacc->second, id, message_error, cacc->second.top(), msg));
		var.notify_all();
	}
}

void logger::fatal(std::string msg) {
	if(emit_level >= 0) {
		std::thread::id id = std::this_thread::get_id();
		id_int_map::accessor lacc;
		id_stack_str_map::accessor cacc;
		bool found = thread_indent_levels.find(lacc, id);
		assert(found);
		found = thread_current_context.find(cacc, id);
		assert(found);
		assert(cacc->second.size() > 0);
		message_q.push(message(lacc->second, id, message_fatal, cacc->second.top(), msg));
		var.notify_all();
	}
}

void logger::msg(message msg) {
	message_q.push(msg);
	var.notify_all();
}

void logger::set_emit_level(int emit) {
	emit_level = emit;
}

void logger::logging_thread() {
	LOG_BEGIN_THIS_THREAD(LOG);
	std::cout << "Log format: HH:MM:SS [thread/context/level] message" << std::endl << std::endl;
	output_message(message(0, std::this_thread::get_id(), message_info, "LOGGING", "Started logging thread."));
	while(!end_thread) {
		std::unique_lock<std::mutex> lock(log_mutex);
		var.wait(lock);
		message m;
		while(message_q.try_pop(m)) {
			output_message(m);
		}
	}
	output_message(message(0, std::this_thread::get_id(), message_info, "LOGGING", "Ended logging thread."));
}

void logger::output_message(message msg) {
	for(std::ostream* out : out_streams) {
		std::time_t t(0);
		time(&t);
		struct tm buf;
#ifdef _WIN32
		localtime_s(&buf, &t);
#else
		localtime_r(&t, &buf);
#endif
		*out << std::put_time(&buf, "%H:%M:%S");
		for(int i = 0; i < msg.indent_level; i++) {
			*out << "\t";
		}
		id_str_map::accessor nacc;
		bool found = thread_names.find(nacc, msg.id);
		if(found)
			*out << " [" << nacc->second << "/" << msg.context << "/" << LEVEL_STR(msg.lvl) << "] " << msg.msg << std::endl;
	}
}

