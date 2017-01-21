
#pragma once

#include "..\common.hpp"

#include <thread>
#include <stack>
#include <ostream>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>

enum message_level {
	message_info  = 3,
	message_warn  = 2,
	message_error = 1,
	message_fatal = 0
};

struct message {
	message();
	message(int indent, std::thread::id id, message_level level, std::string c, std::string m);
	
	std::thread::id id;
	int indent_level;
	message_level lvl;
	std::string context;
	std::string msg;
};

typedef tbb::concurrent_hash_map<std::thread::id, int> id_int_map;
typedef tbb::concurrent_hash_map<std::thread::id, std::string> id_str_map;
typedef tbb::concurrent_hash_map<std::thread::id, std::stack<std::string>> id_stack_str_map;

class logger {
public:
	logger(int emit);
	~logger();

	void begin_on_thread(std::string name);
	
	void pushSec();
	void popSec ();

	void pushContext(std::string context);
	void popContext ();

	void info (std::string msg);
	void warn (std::string msg);
	void error(std::string msg);
	void fatal(std::string msg);
	void msg  (message msg);

	void set_emit_level(int emit);

private:
	void logging_thread();
	void output_message(message msg);

	std::thread* 			thread;
	std::mutex   			log_mutex;
	std::condition_variable var;
	bool 					end_thread;

	int emit_level;

	id_int_map					 					thread_indent_levels;
	std::vector<std::ostream*, alloc<std::ostream*>>		out_streams;
	tbb::concurrent_queue<message, alloc<message>> 	message_q;

	id_str_map			thread_names;
	id_stack_str_map 	thread_current_context;
};

extern logger glog;

#if EMIT_LEVEL != 0
#define LOG_BEGIN_THIS_THREAD(str)	glog.begin_on_thread(#str);
#define LOG_PUSH_SEC() 				glog.pushSec();
#define LOG_POP_SEC()  				glog.popSec();
#define LOG_PUSH_CONTEXT(str)		glog.pushContext(#str);
#define LOG_POP_CONTEXT()			glog.popContext();
#define LOG_INFO(str)				glog.info(str);
#define LOG_WARN(str)				glog.warn(str);
#define LOG_ERROR(str)				glog.error(str);
#define LOG_FATAL(str)				glog.fatal(str);
#define LOG_MSG_RW(msg)				glog.msg(msg);
#define LOG_SET_EMIT_LVL(emit)		glog.set_emit_level(emit);
#else
#define LOG_BEGIN_THIS_THREAD(str)	
#define LOG_PUSH_SEC() 				
#define LOG_POP_SEC()  				
#define LOG_PUSH_CONTEXT(str)		
#define LOG_POP_CONTEXT()			
#define LOG_INFO(str)				
#define LOG_WARN(str)				
#define LOG_ERROR(str)				
#define LOG_FATAL(str)				
#define LOG_MSG_RW(msg)				
#define LOG_SET_EMIT_LVL(emit)		
#endif