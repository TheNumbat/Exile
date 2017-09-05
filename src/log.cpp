
log_manager log_manager::make(allocator* a) { PROF

	log_manager ret;

	ret.out = vector<log_out>::make(4, a);
	ret.message_queue = con_queue<log_message>::make(8, a);
	global_api->platform_create_semaphore(&ret.logging_semaphore, 0, INT32_MAX);

	ret.alloc = a;
	ret.scratch = MAKE_ARENA("log scratch", KILOBYTES(512), a, true);

	return ret;
}

void log_manager::start() { PROF

	thread_param.out 				= &out;
	thread_param.message_queue 		= &message_queue;
	thread_param.logging_semaphore 	= &logging_semaphore;
	thread_param.running 			= true;
	thread_param.alloc 				= alloc;
	thread_param.scratch			= &scratch;

	global_api->platform_create_thread(&logging_thread, &log_proc, &thread_param, false);
}

void log_manager::stop() { PROF

	thread_param.running = false;

	global_api->platform_signal_semaphore(&logging_semaphore, 1);
	global_api->platform_join_thread(&logging_thread, -1);
	global_api->platform_destroy_thread(&logging_thread);

	thread_param.out 				= null;
	thread_param.message_queue 		= null;
	thread_param.logging_semaphore 	= null;
	thread_param.alloc 				= null;
	thread_param.scratch			= null;
}

void log_manager::destroy() { PROF

	if(thread_param.running) {
		stop();
	}

	FORVEC(out,
		print_footer(it);
		if(it->type != log_out_type::custom) {
			it->file.destroy();
		}
	)

	out.destroy();
	message_queue.destroy();
	global_api->platform_destroy_semaphore(&logging_semaphore);
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

void log_manager::add_file(platform_file file, log_level level, log_out_type type) { PROF

	log_out lfile;
	lfile.type = type;
	lfile.file = buffer<platform_file,1024>::make(write_file_wrapper, file);
	lfile.level = level;
	
	print_header(&lfile);
	out.push(lfile);
}

void log_manager::add_output(log_out output) { PROF

	if(output.type != log_out_type::custom) {
		print_header(&output);
	}

	out.push(output);
}

void log_manager::print_header(log_out* output) { PROF

	PUSH_ALLOC(alloc) {
		
		if(output->type == log_out_type::plaintext) {
			
			string header = string::makef(string_literal("%-8 [%-36] [%-20] [%-5] %-2\n"), string_literal("time"), string_literal("thread/context"), string_literal("file:line"), string_literal("level"), string_literal("message"));

			output->file.write((void*)header.c_str, header.len - 1);
			output->file.flush();

			header.destroy();

		} else if(output->type == log_out_type::html) {

			output->file.write((void*)log_html_header.c_str, log_html_header.len - 1);
			output->file.flush();
		}

	} POP_ALLOC();
}

void log_manager::print_footer(log_out* output) {

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
	arena_allocator arena = MAKE_ARENA("msg", arena_size, alloc, true);

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

		if(level == log_level::error) {
			if(global_api->platform_is_debugging()) {
				global_api->platform_debug_break();
			}
#ifdef BLOCK_ON_ERROR
			global_api->platform_join_thread(&logging_thread, -1);
#endif
		}
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
	arena_allocator arena = MAKE_ARENA("msg", arena_size, alloc, true);

	PUSH_ALLOC(&arena) {

		lmsg.msg = string::make_copy(msg);
		lmsg.publisher = context;
		lmsg.level = level;

		lmsg.call_stack = array<code_context>::make_memory(this_thread_data.call_stack_depth, malloc(sizeof(code_context) * this_thread_data.call_stack_depth));
		lmsg.thread_name = string::make_copy(this_thread_data.name);
		memcpy(this_thread_data.call_stack, lmsg.call_stack.memory, sizeof(code_context) * this_thread_data.call_stack_depth);
		
		lmsg.arena = arena;
		message_queue.push(lmsg);

		global_api->platform_signal_semaphore(&logging_semaphore, 1);

		if(level == log_level::error) {
			if(global_api->platform_is_debugging()) {
				global_api->platform_debug_break();
			}
#ifdef BLOCK_ON_ERROR
			global_api->platform_join_thread(&logging_thread, -1);
#endif
		}
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
	global_api->platform_get_timef(string_literal("hh:mm:ss"), &time);

	return time;
}

string log_message::fmt_call_stack() { PROF

	string cstack = string::make_cat(thread_name, string_literal("/"));
	for(u32 j = 0; j < call_stack.capacity; j++) {
		string temp = string::make_cat_v(3, cstack, call_stack.get(j)->function, string_literal("/"));
		cstack.destroy();
		cstack = temp;
	}

	return cstack;
}

string log_message::fmt_file_line() { PROF

	return string::makef(string_literal("%:%"), publisher.file, publisher.line);
}

string log_message::fmt_level() { PROF

	string str;
	switch(level) {
	case log_level::debug:
		str = string_literal("DEBUG");
		break;
	case log_level::info:
		str = string_literal("INFO");
		break;
	case log_level::warn:
		str = string_literal("WARN");
		break;
	case log_level::error:
		str = string_literal("ERROR");
		break;
	case log_level::fatal:
		str = string_literal("FATAL");
		break;
	case log_level::ogl:
		str = string_literal("OGL");
		break;
	case log_level::alloc:
		str = string_literal("ALLOC");
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

		output = string::makef(string_literal("%-8 [%-36] [%-20] [%-5] %+*\n"), time, cstack, file_line, clevel, 3 * msg->call_stack.capacity + msg->msg.len - 1, msg->msg);

	} else if(type == log_out_type::html) {

		output = string::makef(log_html_msg, time, cstack, file_line, clevel, msg->msg);
	}

	time.destroy();
	cstack.destroy();
	file_line.destroy();

	return output;	
}

i32 log_proc(void* data_) {

	log_thread_param* data = (log_thread_param*)data_;	

	begin_thread(np_string_literal("log"), data->alloc);

	while(data->running) {

		log_message msg;
		while(data->message_queue->try_pop(&msg)) {

			if(msg.msg.c_str != null) {
				
				string output;
				PUSH_ALLOC(data->scratch) {
					
					FORVEC(*data->out,

						output = fmt_msg(&msg, it->type);

						if(it->level <= msg.level) {
							if(it->type == log_out_type::custom) {
								it->write(&msg);
							} else {
								it->file.write(output.c_str, output.len - 1);
							}
						}

						output.destroy();
					)

				} POP_ALLOC();
				RESET_ARENA(data->scratch);

				if(msg.level == log_level::fatal) {
					// die
					exit(1);
				}

				DESTROY_ARENA(&msg.arena);
			}
		}

		global_api->platform_wait_semaphore(data->logging_semaphore, -1);
	}

	end_thread();

	return 0;
}
