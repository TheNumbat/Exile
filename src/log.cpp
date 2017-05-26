
logger make_logger(allocator* a) {

	logger ret;

	ret.out = make_vector<log_file>(4, a);
	ret.message_queue = make_queue<log_message>(8, a);
	global_state->api->platform_create_mutex(&ret.queue_mutex, false);
	global_state->api->platform_create_mutex(&ret.thread_data_mutex, false);
	global_state->api->platform_create_semaphore(&ret.logging_semaphore, 0, UINT32_MAX);
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
}


void logger_init_thread(logger* log, string name, code_context context) {

	log_thread_data this_data;

	this_data.context_name = make_stack<string>(8, log->alloc);
	this_data.name = name;
	this_data.start_context = context;

	global_state->api->platform_aquire_mutex(&log->thread_data_mutex, -1);
	map_insert_if_unique(&log->thread_data, global_state->api->platform_this_thread_id(), this_data);
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
	log->alloc = NULL;
}

i32 logging_thread(void* data_) {

	log_thread_param* data = (log_thread_param*)data_;	

	return 0;
}
