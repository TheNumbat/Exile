
log_manager make_logger(allocator* a) { FUNC

	log_manager ret;

	ret.out = make_vector<log_out>(4, a);
	ret.message_queue = make_queue<log_message>(8, a);
	global_state->api->platform_create_mutex(&ret.queue_mutex, false);
	global_state->api->platform_create_semaphore(&ret.logging_semaphore, 0, INT32_MAX);

	ret.alloc = a;
	ret.scratch = MAKE_ARENA("log scratch", 2048, a, true);

	return ret;
}

void logger_start(log_manager* log) { FUNC

	log->thread_param.out 				= &log->out;
	log->thread_param.message_queue 	= &log->message_queue;
	log->thread_param.queue_mutex		= &log->queue_mutex;
	log->thread_param.logging_semaphore = &log->logging_semaphore;
	log->thread_param.running 			= true;
	log->thread_param.alloc 			= log->alloc;
	log->thread_param.scratch			= &log->scratch;

	global_state->api->platform_create_thread(&log->logging_thread, &logging_thread, &log->thread_param, false);
}

void logger_stop(log_manager* log) { FUNC

	log->thread_param.running = false;

	global_state->api->platform_signal_semaphore(&log->logging_semaphore, 1);
	global_state->api->platform_join_thread(&log->logging_thread, -1);
	global_state->api->platform_destroy_thread(&log->logging_thread);

	log->thread_param.out 				= NULL;
	log->thread_param.message_queue 	= NULL;
	log->thread_param.queue_mutex		= NULL;
	log->thread_param.logging_semaphore = NULL;
	log->thread_param.alloc 			= NULL;
	log->thread_param.scratch			= NULL;
}

void destroy_logger(log_manager* log) { FUNC

	if(log->thread_param.running) {
		logger_stop(log);
	}

	destroy_vector(&log->out);
	destroy_queue(&log->message_queue);
	global_state->api->platform_destroy_mutex(&log->queue_mutex);
	global_state->api->platform_destroy_semaphore(&log->logging_semaphore);
	DESTROY_ARENA(&log->scratch);
	log->alloc = NULL;
}

void logger_push_context(log_manager* log, string context, code_context fake) { FUNC_NOCS

	fake.function = context;

	LOG_DEBUG_ASSERT(this_thread_data.call_stack_depth < MAX_CALL_STACK_DEPTH);
	this_thread_data.call_stack[this_thread_data.call_stack_depth++] = fake;
}

void logger_pop_context(log_manager* log) { FUNC_NOCS

	this_thread_data.call_stack_depth--;
}

void logger_add_file(log_manager* log, platform_file file, log_level level) { FUNC

	log_out lfile;
	lfile.file = file;
	lfile.level = level;
	vector_push(&log->out, lfile);

	logger_print_header(log, lfile);
}

void logger_add_output(log_manager* log, log_out out) { FUNC

	vector_push(&log->out, out);
	if(!out.custom) {
		logger_print_header(log, out);
	}
}

void logger_print_header(log_manager* log, log_out out) { FUNC

	PUSH_ALLOC(log->alloc) {
		
		string header = make_stringf(string_literal("%-8 [%-36] [%-20] [%-5] %-2\r\n"), string_literal("time"), string_literal("thread/context"), string_literal("file:line"), string_literal("level"), string_literal("message"));

		global_state->api->platform_write_file(&out.file, (void*)header.c_str, header.len - 1);

		free_string(header);

	} POP_ALLOC();
}

template<typename... Targs> 
void logger_msgf(log_manager* log, string fmt, log_level level, code_context context, Targs... args) { FUNC_NOCS

	log_message lmsg;

	lmsg.arena = MAKE_ARENA("msg arena", 1024, log->alloc, true);
	PUSH_ALLOC(&lmsg.arena) {

		lmsg.msg = make_stringf(fmt, args...);

		lmsg.publisher = context;
		lmsg.level = level;

		lmsg.call_stack = make_array<code_context>(this_thread_data.call_stack_depth, &lmsg.arena);
		lmsg.thread_name = make_copy_string(this_thread_data.name);
		memcpy(this_thread_data.call_stack, lmsg.call_stack.memory, sizeof(code_context) * this_thread_data.call_stack_depth);
		
		global_state->api->platform_aquire_mutex(&log->queue_mutex, -1);
		queue_push(&log->message_queue, lmsg);
		global_state->api->platform_release_mutex(&log->queue_mutex);
		global_state->api->platform_signal_semaphore(&log->logging_semaphore, 1);

		if(level == log_level::error) {
			if(global_state->api->platform_is_debugging()) {
				__debugbreak();	
			}
#ifdef BLOCK_ON_ERROR
			global_state->api->platform_join_thread(&log->logging_thread, -1);
#endif
		}
		if(level == log_level::fatal) {
			if(global_state->api->platform_is_debugging()) {
				__debugbreak();	
			}
			global_state->api->platform_join_thread(&log->logging_thread, -1);
		}

	} POP_ALLOC();
}

void logger_msg(log_manager* log, string msg, log_level level, code_context context) { FUNC_NOCS

	log_message lmsg;

	lmsg.arena = MAKE_ARENA("msg arena", 1024, log->alloc, true);
	PUSH_ALLOC(&lmsg.arena) {

		lmsg.msg = make_copy_string(msg);
		lmsg.publisher = context;
		lmsg.level = level;

		lmsg.call_stack = make_array<code_context>(this_thread_data.call_stack_depth, &lmsg.arena);
		lmsg.thread_name = make_copy_string(this_thread_data.name);
		memcpy(this_thread_data.call_stack, lmsg.call_stack.memory, sizeof(code_context) * this_thread_data.call_stack_depth);
		
		global_state->api->platform_aquire_mutex(&log->queue_mutex, -1);
		queue_push(&log->message_queue, lmsg);
		global_state->api->platform_release_mutex(&log->queue_mutex);
		global_state->api->platform_signal_semaphore(&log->logging_semaphore, 1);

		if(level == log_level::error) {
			if(global_state->api->platform_is_debugging()) {
				__debugbreak();	
			}
#ifdef BLOCK_ON_ERROR
			global_state->api->platform_join_thread(&log->logging_thread, -1);
#endif
		}
		if(level == log_level::fatal) {
			if(global_state->api->platform_is_debugging()) {
				__debugbreak();	
			}
			global_state->api->platform_join_thread(&log->logging_thread, -1);
		}

	} POP_ALLOC();
}

string log_fmt_msg_time(log_message* msg) { FUNC

	string time = make_string(9);
	global_state->api->platform_get_timef(string_literal("hh:mm:ss"), &time);

	return time;
}

string log_fmt_msg_call_stack(log_message* msg) { FUNC_NOCS

	string call_stack = make_cat_string(msg->thread_name, string_literal("/"));
	for(u32 j = 0; j < msg->call_stack.capacity; j++) {
		string temp = make_cat_strings(3, call_stack, array_get(&msg->call_stack, j)->function, string_literal("/"));
		free_string(call_stack);
		call_stack = temp;
	}

	return call_stack;
}

string log_fmt_msg_file_line(log_message* msg) { FUNC

	return make_stringf(string_literal("%:%"), msg->publisher.file, msg->publisher.line);
}

string log_fmt_msg_level(log_message* msg) { FUNC

	string level;
	switch(msg->level) {
	case log_level::debug:
		level = string_literal("DEBUG");
		break;
	case log_level::info:
		level = string_literal("INFO");
		break;
	case log_level::warn:
		level = string_literal("WARN");
		break;
	case log_level::error:
		level = string_literal("ERROR");
		break;
	case log_level::fatal:
		level = string_literal("FATAL");
		break;
	case log_level::ogl:
		level = string_literal("OGL");
		break;
	case log_level::alloc:
		level = string_literal("ALLOC");
		break;
	}

	return level;
}

string log_fmt_msg(log_message* msg) { FUNC

	string time = log_fmt_msg_time(msg);
	string call_stack = log_fmt_msg_call_stack(msg);
	string file_line = log_fmt_msg_file_line(msg);
	string level = log_fmt_msg_level(msg);

	string output = make_stringf(string_literal("%-8 [%-36] [%-20] [%-5] %+*\r\n"), time, call_stack, file_line, level, 3 * msg->call_stack.capacity + msg->msg.len - 1, msg->msg);

	free_string(time);
	free_string(call_stack);
	free_string(file_line);

	return output;	
}

void logger_rem_output(log_manager* log, log_out out) { FUNC

	vector_erase(&log->out, out);
}

bool operator==(log_out l, log_out r) { FUNC
	if(!(l.level == r.level && l.custom == r.custom)) return false;
	if(l.custom && l.write == r.write) return true;
	return l.file == r.file;
}

i32 logging_thread(void* data_) { FUNC_NOCS

	log_thread_param* data = (log_thread_param*)data_;	

	begin_thread(string_literal("log"), &global_state->suppressed_platform_allocator);

	while(data->running) {

		for(;;) {
		
			global_state->api->platform_aquire_mutex(data->queue_mutex, -1);

			if(queue_empty(data->message_queue)) {
				global_state->api->platform_release_mutex(data->queue_mutex);
				break;
			}
			log_message msg = queue_pop(data->message_queue);
			
			global_state->api->platform_release_mutex(data->queue_mutex);

			if(msg.msg.c_str != NULL) {
				
				string output;
				PUSH_ALLOC(data->scratch) {
					
					output = log_fmt_msg(&msg);
				
					FORVEC(*data->out,
						if(it->level <= msg.level) {
							if(it->custom) {
								it->write(&msg);
							} else {
								global_state->api->platform_write_file(&it->file, (void*)output.c_str, output.len - 1);
							}
						}
					)

					free_string(output);

				} POP_ALLOC();
				RESET_ARENA(data->scratch);

				if(msg.level == log_level::fatal) {
					// die
					exit(1);
				}

				DESTROY_ARENA(&msg.arena);
			}
		}

		global_state->api->platform_wait_semaphore(data->logging_semaphore, -1);
	}

	end_thread();

	return 0;
}
