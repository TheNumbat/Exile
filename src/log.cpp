
log_manager log_manager::make(allocator* a) { PROF

	log_manager ret;

	ret.out = vector<log_out>::make(4, a);
	ret.message_queue = locking_queue<log_message>::make(32, a);
	CHECKED(platform_create_semaphore, &ret.logging_semaphore, 0, INT32_MAX);
	global_api->platform_create_mutex(&ret.output_mut, false);

	ret.alloc = a;
	ret.scratch = MAKE_ARENA("log scratch"_, MEGABYTES(1), a, true);

	return ret;
}

void log_manager::start() { PROF

	thread_param.out 				= &out;
	thread_param.message_queue 		= &message_queue;
	thread_param.logging_semaphore 	= &logging_semaphore;
	thread_param.running 			= true;
	thread_param.alloc 				= alloc;
	thread_param.scratch			= &scratch;
	thread_param.output_mut 		= &output_mut;

	CHECKED(platform_create_thread, &logging_thread, &log_proc, &thread_param, false);
}

void log_manager::stop() { PROF

	thread_param.running = false;

	CHECKED(platform_signal_semaphore, &logging_semaphore, 1);
	global_api->platform_join_thread(&logging_thread, -1);
	CHECKED(platform_destroy_thread, &logging_thread);

	thread_param.out 				= null;
	thread_param.message_queue 		= null;
	thread_param.logging_semaphore 	= null;
	thread_param.alloc 				= null;
	thread_param.scratch			= null;
	thread_param.output_mut 		= null;
}

void log_manager::destroy() { PROF

	if(thread_param.running) {
		stop();
	}

	FORVEC(it, out) {
		print_footer(it);
		if(it->type != log_out_type::custom) {
			it->file.destroy();
		}
	}
	// NOTE(max): just to be absolutely sure nothing slipped through
	FORQ_BEGIN(it, message_queue) {
		DESTROY_ARENA(&it->arena);
	} FORQ_END(it, message_queue);

	out.destroy();
	message_queue.destroy();
	CHECKED(platform_destroy_semaphore, &logging_semaphore);
	global_api->platform_destroy_mutex(&output_mut);
	DESTROY_ARENA(&scratch);
	alloc = null;
}

void log_manager::push_context(string context, code_context fake) { PROF_NOCS

	fake.function = context;

	LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth < MAX_CALL_STACK_DEPTH);
	this_thread_data.call_stack[this_thread_data.call_stack_depth++] = fake;
}

void log_manager::pop_context() { PROF_NOCS

	LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth > 0);
	this_thread_data.call_stack_depth--;
}

void log_manager::add_file(platform_file file, log_level level, log_out_type type, bool flush) { PROF

	log_out lfile;
	lfile.type = type;
	lfile.flush_on_message = flush;
	lfile.file = buffer<platform_file,4096>::make(FPTR(write_file_wrapper), file);
	lfile.level = level;
	
	print_header(&lfile);
	out.push(lfile);
}

void log_manager::add_custom_output(log_out output) { PROF

	if(output.type != log_out_type::custom) {
		print_header(&output);
	}

	global_api->platform_aquire_mutex(&output_mut);
	out.push(output);
	global_api->platform_release_mutex(&output_mut);
}

void log_manager::rem_custom_output(log_out output) { PROF

	global_api->platform_aquire_mutex(&output_mut);
	out.erase(output);
	global_api->platform_release_mutex(&output_mut);
}

void log_manager::print_header(log_out* output) { PROF

	PUSH_ALLOC(alloc) {
		
		if(output->type == log_out_type::plaintext) {
			
			string header = string::makef("%-8 [%-36] [%-20] [%-5] %-2\n"_, "time"_, "thread/context"_, "file:line"_, "level"_, "message"_);

			output->file.write((void*)header.c_str, header.len - 1);
			output->file.flush();

			header.destroy();

		} else if(output->type == log_out_type::html) {

			output->file.write((void*)log_html_header.c_str, log_html_header.len - 1);
			output->file.flush();
		}

	} POP_ALLOC();
}

void log_manager::print_footer(log_out* output) { PROF

	PUSH_ALLOC(alloc) {

		if(output->type == log_out_type::html) {
			
			output->file.write((void*)log_html_footer.c_str, log_html_footer.len - 1);
			output->file.flush();
		}

	} POP_ALLOC();
}

template<typename... Targs> 
void log_manager::msgf(string fmt, log_level level, code_context context, Targs... args) { PROF_NOCS

	log_message lmsg;

	u32 msg_len = size_stringf(fmt, args...);
	u32 arena_size = msg_len + this_thread_data.name.len + this_thread_data.call_stack_depth * sizeof(code_context);
	arena_allocator arena = MAKE_ARENA("msg"_, arena_size, alloc, true);

	PUSH_ALLOC(&arena) {

		lmsg.msg = string::makef(msg_len, fmt, args...);

		lmsg.publisher = context;
		lmsg.level = level;

		lmsg.call_stack = array<code_context>::make_memory(this_thread_data.call_stack_depth, malloc(sizeof(code_context) * this_thread_data.call_stack_depth));
		lmsg.thread_name = string::make_copy(this_thread_data.name);
		memcpy(this_thread_data.call_stack, lmsg.call_stack.memory, sizeof(code_context) * this_thread_data.call_stack_depth);

		lmsg.arena = arena;
		message_queue.push(lmsg);
		
		global_api->platform_signal_semaphore(&logging_semaphore, 1);

#ifdef BLOCK_OR_EXIT_ON_ERROR
		if(level == log_level::error) {

			if(global_api->platform_is_debugging()) {
				global_api->platform_debug_break();
			}
			global_api->platform_join_thread(&logging_thread, -1);
		}
#endif
		if(level == log_level::fatal) {
			if(global_api->platform_is_debugging()) {
				global_api->platform_debug_break();
			}
			global_api->platform_join_thread(&logging_thread, -1);
		}

	} POP_ALLOC();
}

void log_manager::msg(string msg, log_level level, code_context context) { PROF_NOCS

	log_message lmsg;

	u32 arena_size = msg.len + this_thread_data.name.len + this_thread_data.call_stack_depth * sizeof(code_context);
	arena_allocator arena = MAKE_ARENA("msg"_, arena_size, alloc, true);

	PUSH_ALLOC(&arena) {

		lmsg.msg = string::make_copy(msg);
		lmsg.publisher = context;
		lmsg.level = level;

		lmsg.call_stack = array<code_context>::make_memory(this_thread_data.call_stack_depth, malloc(sizeof(code_context) * this_thread_data.call_stack_depth));
		lmsg.thread_name = string::make_copy(this_thread_data.name);
		memcpy(this_thread_data.call_stack, lmsg.call_stack.memory, sizeof(code_context) * this_thread_data.call_stack_depth);
		
		lmsg.arena = arena;
		message_queue.push(lmsg);

		CHECKED(platform_signal_semaphore, &logging_semaphore, 1);

#ifdef BLOCK_OR_EXIT_ON_ERROR
		if(level == log_level::error) {

			if(global_api->platform_is_debugging()) {
				global_api->platform_debug_break();
			}
			global_api->platform_join_thread(&logging_thread, -1);
		}
#endif
		if(level == log_level::fatal) {
			if(global_api->platform_is_debugging()) {
				global_api->platform_debug_break();
			}
			global_api->platform_join_thread(&logging_thread, -1);
		}

	} POP_ALLOC();
}

string log_message::fmt_time() { PROF

	string time = string::make(9);
	global_api->platform_get_timef("hh:mm:ss"_, &time);

	return time;
}

string log_message::fmt_call_stack() { PROF

	string cstack = string::make_cat(thread_name, "/"_);
	for(u32 j = 0; j < call_stack.capacity; j++) {
		string temp = string::make_cat_v(3, cstack, call_stack.get(j)->function, "/"_);
		cstack.destroy();
		cstack = temp;
	}

	return cstack;
}

string log_message::fmt_file_line() { PROF

	return string::makef("%:%"_, publisher.file, publisher.line);
}

string log_message::fmt_level() { PROF

	string str;
	switch(level) {
	case log_level::debug:
		str = "DEBUG"_;
		break;
	case log_level::info:
		str = "INFO"_;
		break;
	case log_level::warn:
		str = "WARN"_;
		break;
	case log_level::error:
		str = "ERROR"_;
		break;
	case log_level::fatal:
		str = "FATAL"_;
		break;
	case log_level::ogl:
		str = "OGL"_;
		break;
	case log_level::alloc:
		str = "ALLOC"_;
		break;
	}

	return str;
}

string fmt_msg(log_message* msg, log_out_type type) { PROF

	string time = msg->fmt_time();
	string cstack = msg->fmt_call_stack();
	string file_line = msg->fmt_file_line();
	string clevel = msg->fmt_level();

	string output;

	if(type == log_out_type::plaintext) {

		output = string::makef("%-8 [%-36] [%-20] [%-5] %\n"_, time, cstack, file_line, clevel, msg->msg);

	} else if(type == log_out_type::html) {

		output = string::makef(log_html_msg, time, cstack, file_line, clevel, msg->msg);
	}

	time.destroy();
	cstack.destroy();
	file_line.destroy();

	return output;	
}

bool operator==(log_out l, log_out r) {

	return l.level == r.level && r.type == r.type && l.flush_on_message == r.flush_on_message && r.param == l.param;
}

i32 log_proc(void* data_) {

	log_thread_param* data = (log_thread_param*)data_;	

	begin_thread("log"_, data->alloc);

	while(data->running) {

		global_api->platform_wait_semaphore(data->logging_semaphore, -1);

		log_message msg;
		while(data->message_queue->try_pop(&msg)) {
			do_msg(data, msg);
		}
	}

	FORVEC(it, *data->out) {
		if(it->type != log_out_type::custom) {
			it->file.flush();
		}
	}
	end_thread();

	return 0;
}

void do_msg(log_thread_param* data, log_message msg) {

	if(msg.msg.c_str != null) {
		
		PUSH_ALLOC(data->scratch) {
			
			global_api->platform_aquire_mutex(data->output_mut);
			FORVEC(it, *data->out) {

				if(it->level <= msg.level) {
					if(it->type == log_out_type::custom) {
						it->write(&msg, it->param);
					} else {
						string output = fmt_msg(&msg, it->type);
						it->file.write(output.c_str, output.len - 1);
						if(it->flush_on_message)
							it->file.flush();
						output.destroy();
					}
				}
			}
			global_api->platform_release_mutex(data->output_mut);

		} POP_ALLOC();
		RESET_ARENA(data->scratch);

#ifdef BLOCK_OR_EXIT_ON_ERROR
		if(msg.level == log_level::error) {
			exit(1);
		}
#endif
		if(msg.level == log_level::fatal) {
			exit(1);
		}

		DESTROY_ARENA(&msg.arena);
	}
}
