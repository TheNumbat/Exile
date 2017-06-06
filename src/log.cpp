
logger make_logger(allocator* a) {

	logger ret;

	ret.out = make_vector<log_file>(4, a);
	ret.message_queue = make_queue<log_message>(8, a);
	global_state->api->platform_create_mutex(&ret.queue_mutex, false);
	global_state->api->platform_create_mutex(&ret.thread_data_mutex, false);
	global_state->api->platform_create_semaphore(&ret.logging_semaphore, 0, INT32_MAX);
	ret.thread_data = make_map<platform_thread_id,log_thread_data>(8, a);
	ret.alloc = a;
	ret.scratch = MAKE_ARENA("log scratch", 2048, a, true);

	return ret;
}

void logger_start(logger* log) {

	log->thread_param.out 				= &log->out;
	log->thread_param.message_queue 	= &log->message_queue;
	log->thread_param.queue_mutex		= &log->queue_mutex;
	log->thread_param.logging_semaphore = &log->logging_semaphore;
	log->thread_param.running 			= true;
	log->thread_param.alloc 			= log->alloc;
	log->thread_param.scratch			= &log->scratch;

	global_state->api->platform_create_thread(&log->logging_thread, &logging_thread, &log->thread_param, false);
}

void logger_stop(logger* log) {

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

void logger_end_thread(logger* log) {

	global_state->api->platform_aquire_mutex(&log->thread_data_mutex, -1);
	
	log_thread_data* data = map_get(&log->thread_data, global_state->api->platform_this_thread_id());
	destroy_stack(&data->context_name);
	map_erase(&log->thread_data, global_state->api->platform_this_thread_id());

	global_state->api->platform_release_mutex(&log->thread_data_mutex);
}

void logger_init_thread(logger* log, string name, code_context context) {

	log_thread_data this_data;

	this_data.context_name = make_stack<string>(8, log->alloc);
	this_data.name = name;
	this_data.start_context = context;

	global_state->api->platform_aquire_mutex(&log->thread_data_mutex, -1);
	map_insert(&log->thread_data, global_state->api->platform_this_thread_id(), this_data);
	global_state->api->platform_release_mutex(&log->thread_data_mutex);
}

void destroy_logger(logger* log) {

	if(log->thread_param.running) {
		logger_stop(log);
	}

	destroy_vector(&log->out);
	destroy_queue(&log->message_queue);
	global_state->api->platform_destroy_mutex(&log->queue_mutex);
	global_state->api->platform_destroy_mutex(&log->thread_data_mutex);
	global_state->api->platform_destroy_semaphore(&log->logging_semaphore);
	destroy_map(&log->thread_data);
	DESTROY_ARENA(&log->scratch);
	log->alloc = NULL;
}

void logger_push_context(logger* log, string context) {

	global_state->api->platform_aquire_mutex(&log->thread_data_mutex, -1);
	log_thread_data* data = map_get(&log->thread_data, global_state->api->platform_this_thread_id());
	stack_push(&data->context_name, context);
	global_state->api->platform_release_mutex(&log->thread_data_mutex);
}

void logger_pop_context(logger* log) {

	global_state->api->platform_aquire_mutex(&log->thread_data_mutex, -1);
	log_thread_data* data = map_get(&log->thread_data, global_state->api->platform_this_thread_id());
	stack_pop(&data->context_name);
	global_state->api->platform_release_mutex(&log->thread_data_mutex);
}

void logger_add_file(logger* log, platform_file file, log_level level) {

	log_file lfile;
	lfile.file = file;
	lfile.level = level;
	vector_push(&log->out, lfile);

	logger_print_header(log, lfile);
}

void logger_print_header(logger* log, log_file file) {

	PUSH_ALLOC(log->alloc) {
		
		string header = make_stringf(string_literal("%-8s [%-24s] [%-20s] [%-5s] %-2s\r\n"), "time", "thread/context", "file:line", "level", "message");

		global_state->api->platform_write_file(&file.file, (void*)header.c_str, header.len - 1);

		free_string(header);

	} POP_ALLOC();
}

void logger_msgf(logger* log, string fmt, log_level level, code_context context, ...) {

	log_message lmsg;

	lmsg.arena = MAKE_ARENA("msg arena", 512, log->alloc, true);
	PUSH_ALLOC(&lmsg.arena) {

		va_list args;
		va_start(args, context);
		lmsg.msg = make_vstringf(fmt, args);
		va_end(args);

		lmsg.publisher = context;
		lmsg.level = level;

		global_state->api->platform_aquire_mutex(&log->thread_data_mutex, -1);
		lmsg.data = *map_get(&log->thread_data, global_state->api->platform_this_thread_id());
		lmsg.data.context_name = make_stack_copy(lmsg.data.context_name, &lmsg.arena);
		lmsg.data.name = make_copy_string(lmsg.data.name);
		global_state->api->platform_release_mutex(&log->thread_data_mutex);

		global_state->api->platform_aquire_mutex(&log->queue_mutex, -1);
		queue_push(&log->message_queue, lmsg);
		global_state->api->platform_release_mutex(&log->queue_mutex);
		global_state->api->platform_signal_semaphore(&log->logging_semaphore, 1);

#ifdef BREAK_ERROR
		if(level == log_error) {
			__debugbreak();
			global_state->api->platform_wait_semaphore(data->logging_semaphore, -1);
		}
#endif
		if(level == log_fatal) {
			// we will never return
			__debugbreak();
			global_state->api->platform_join_thread(&log->logging_thread, -1);
		}

	} POP_ALLOC();
}

void logger_msg(logger* log, string msg, log_level level, code_context context) {

	log_message lmsg;

	lmsg.arena = MAKE_ARENA("msg arena", 256, log->alloc, true);
	PUSH_ALLOC(&lmsg.arena) {

		lmsg.msg = make_copy_string(msg);
		lmsg.publisher = context;
		lmsg.level = level;

		global_state->api->platform_aquire_mutex(&log->thread_data_mutex, -1);
		lmsg.data = *map_get(&log->thread_data, global_state->api->platform_this_thread_id());
		lmsg.data.context_name = make_stack_copy(lmsg.data.context_name, &lmsg.arena);
		lmsg.data.name = make_copy_string(lmsg.data.name);
		global_state->api->platform_release_mutex(&log->thread_data_mutex);

		global_state->api->platform_aquire_mutex(&log->queue_mutex, -1);
		queue_push(&log->message_queue, lmsg);
		global_state->api->platform_release_mutex(&log->queue_mutex);
		global_state->api->platform_signal_semaphore(&log->logging_semaphore, 1);

#ifdef BREAK_ERROR
		if(level == log_error) {
			__debugbreak();
		}
#endif
		if(level == log_fatal) {
			// we will never return
			__debugbreak();
			global_state->api->platform_join_thread(&log->logging_thread, -1);
		}

	} POP_ALLOC();
}

i32 logging_thread(void* data_) {

	log_thread_param* data = (log_thread_param*)data_;	

	global_state->api->platform_aquire_mutex(&global_state->alloc_contexts_mutex, -1);
	map_insert(&global_state->alloc_contexts, global_state->api->platform_this_thread_id(), make_stack<allocator*>(0, data->alloc));
	global_state->api->platform_release_mutex(&global_state->alloc_contexts_mutex);

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
				
				PUSH_ALLOC(data->scratch) {

					string time = make_string(9);
					global_state->api->platform_get_timef(string_literal("hh:mm:ss"), &time);
					
					string thread_contexts = make_cat_string(msg.data.name, string_literal("/"));
					for(u32 j = 0; j < msg.data.context_name.contents.size; j++) {
						string temp = make_cat_strings(3, thread_contexts, *vector_get(&msg.data.context_name.contents, j), string_literal("/"));
						free_string(thread_contexts);
						thread_contexts = temp;
					}

					string file_line = make_stringf(string_literal("%s:%u"), msg.publisher.file.c_str, msg.publisher.line);
					string level;
					switch(msg.level) {
					case log_debug:
						level = string_literal("DEBUG");
						break;
					case log_info:
						level = string_literal("INFO");
						break;
					case log_warn:
						level = string_literal("WARN");
						break;
					case log_error:
						level = string_literal("ERROR");
						break;
					case log_fatal:
						level = string_literal("FATAL");
						break;
					case log_alloc:
						level = string_literal("ALLOC");
						break;
					}

					string final_output = make_stringf(string_literal("%-8s [%-24s] [%-20s] [%-5s] %*s\r\n"), time.c_str, thread_contexts.c_str, file_line.c_str, level.c_str, 3 * msg.data.context_name.contents.size + msg.msg.len - 1, msg.msg.c_str);

					for(u32 i = 0; i < data->out->size; i++) {

						if(vector_get(data->out, i)->level <= msg.level) {

							global_state->api->platform_write_file(&vector_get(data->out, i)->file, (void*)final_output.c_str, final_output.len - 1);
						}
					}

					/*unnecessary, as they are allocated on the scratch arena
					free_string(time);
					free_string(file_line);
					free_string(thread_contexts);
					free_string(final_output);*/

					if(msg.level == log_fatal) {
						// die
						exit(1);
					}
				} POP_ALLOC();
				RESET_ARENA(data->scratch);

				DESTROY_ARENA(&msg.arena);
			}
		}

		global_state->api->platform_wait_semaphore(data->logging_semaphore, -1);
	}

	global_state->api->platform_aquire_mutex(&global_state->alloc_contexts_mutex, -1);
	destroy_stack(map_get(&global_state->alloc_contexts, global_state->api->platform_this_thread_id()));
	map_erase(&global_state->alloc_contexts, global_state->api->platform_this_thread_id());
	global_state->api->platform_release_mutex(&global_state->alloc_contexts_mutex);

	return 0;
}
