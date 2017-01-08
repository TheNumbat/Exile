
#pragma once

#include <thread>
#include <queue>
#include <stack>
#include <map>
#include <ostream>
#include <vector>
#include <mutex>
#include <string>
#include <condition_variable>
// ...that's a _lot_ of STL

enum message_level {
	message_info  = 3,
	message_warn  = 2,
	message_error = 1,
	message_fatal = 0
};

struct message {
	message(int indent, std::thread::id id, message_level level, std::string c, std::string m);
	
	std::thread::id id;
	int indent_level;
	message_level lvl;
	std::string context;
	std::string msg;
};

class logger {
public:
	logger(int emit);
	~logger();

	void begin_on_thread();
	
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

	std::thread* thread;
	std::mutex   q_mutex;
	std::condition_variable var;
	bool end_thread;

	int emit_level;

	std::map<std::thread::id, int> 	thread_indent_levels;
	std::vector<std::ostream*> 		out_streams;
	std::queue<message> 			message_q;

	std::map<std::thread::id, std::stack<std::string>> thread_current_context;
};

extern logger glog;

#define LOG_BEGIN_THIS_THREAD()	glog.begin_on_thread();
#define LOG_PUSH_SEC() 			glog.pushSec();
#define LOG_POP_SEC()  			glog.popSec();
#define LOG_PUSH_CONTEXT(str)	glog.pushContext(#str);
#define LOG_POP_CONTEXT()		glog.popContext();
#define LOG_INFO(str)			glog.info(str);
#define LOG_WARN(str)			glog.warn(str);
#define LOG_ERROR(str)			glog.error(str);
#define LOG_FATAL(str)			glog.fatal(str);
#define LOG_MSG_RW(msg)			glog.msg(msg);
#define LOG_SET_EMIT_LVL(emit)	glog.set_emit_level(emit);

