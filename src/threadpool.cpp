
threadpool make_threadpool(i32 num_threads_) {

	return make_threadpool(CURRENT_ALLOC(), num_threads_);
}

threadpool make_threadpool(allocator* a, i32 num_threads_) {

	// TODO(max): errors

	threadpool ret;

	ret.num_threads = num_threads_ == 0 ? global_platform_api->platform_get_num_cpus() : num_threads_;

	ret.alloc   = a;
	ret.threads = make_array<platform_thread>(ret.num_threads, a);
	ret.data    = make_array<worker_data>(ret.num_threads, a);
	ret.jobs    = make_queue<job>(ret.num_threads, a);

	platform_error err = global_platform_api->platform_create_mutex(&ret.queue_mutex, false);
	if(!err.good) {

	}
	err = global_platform_api->platform_create_semaphore(&ret.jobs_semaphore, 0, ret.num_threads);
	if(!err.good) {
		
	}

	return ret;
}

void destroy_threadpool(threadpool* tp) {

	threadpool_stop_all(tp);

	destroy_array(&tp->threads);
	destroy_array(&tp->data);
	destroy_queue(&tp->jobs);

	global_platform_api->platform_destroy_mutex(&tp->queue_mutex);
	global_platform_api->platform_destroy_semaphore(&tp->jobs_semaphore);
}

void threadpool_queue_job(threadpool* tp, i32 (*proc)(void*), void* data) {

	job j;
	j.proc = proc;
	j.data = data;

	threadpool_queue_job(tp, j);
}

void threadpool_queue_job(threadpool* tp, job j) {

	platform_mutex_state state = global_platform_api->platform_aquire_mutex(&tp->queue_mutex, -1);
	if(!state.error.good) {
		
	}

	queue_push(&tp->jobs, j);

	platform_error err = global_platform_api->platform_release_mutex(&tp->queue_mutex);
	if(!err.good) {
		
	}

	err = global_platform_api->platform_signal_semaphore(&tp->jobs_semaphore, 1);
	if(!err.good) {
		
	}
}

void threadpool_stop_all(threadpool* tp) {

	if(tp->running) {
	
		for(i32 i = 0; i < tp->num_threads; i++) {

			array_get(&tp->data, i).running = false;
		}

		global_platform_api->platform_signal_semaphore(&tp->jobs_semaphore, tp->num_threads);

		for(i32 i = 0; i < tp->num_threads; i++) {

			global_platform_api->platform_join_thread(&array_get(&tp->threads, i), -1);
			global_platform_api->platform_destroy_thread(&array_get(&tp->threads, i));
		}

		tp->running = false;
	}
} 

void threadpool_start_all(threadpool* tp) {

	if(!tp->running) {
	
		for(i32 i = 0; i < tp->num_threads; i++) {

			array_get(&tp->data, i).job_queue 	 	= &tp->jobs;
			array_get(&tp->data, i).queue_mutex 	= &tp->queue_mutex;
			array_get(&tp->data, i).jobs_semaphore  = &tp->jobs_semaphore;
			array_get(&tp->data, i).running 		= true;
			array_get(&tp->data, i).alloc  			= tp->alloc;

			global_platform_api->platform_create_thread(&array_get(&tp->threads, i), &worker, &array_get(&tp->data, i), false);
		}

		tp->running = true;
	}
}

i32 worker(void* data_) {

	worker_data* data = (worker_data*)data_;

	// TODO(max): errors

	global_platform_api->platform_aquire_mutex(global_alloc_contexts_mutex, -1);
	map_insert(global_alloc_contexts, global_platform_api->platform_this_thread_id(), make_stack<allocator*>(0, data->alloc));
	global_platform_api->platform_release_mutex(global_alloc_contexts_mutex);

	while(data->running) {
		job current_job;

		platform_mutex_state state = global_platform_api->platform_aquire_mutex(data->queue_mutex, -1);
		if(!state.error.good) {
			
		}

		if(!queue_empty(data->job_queue)) {
			current_job = queue_pop(data->job_queue);
		}

		platform_error err = global_platform_api->platform_release_mutex(data->queue_mutex);
		if(!err.good) {
			
		}

		if(current_job.proc) {
			(*current_job.proc)(current_job.data);
		}

		

		platform_semaphore_state state_ = global_platform_api->platform_wait_semaphore(data->jobs_semaphore, -1);
		if(!state_.error.good) {
			
		}
	}

	global_platform_api->platform_aquire_mutex(global_alloc_contexts_mutex, -1);
	destroy_stack(&map_get(global_alloc_contexts, global_platform_api->platform_this_thread_id()));
	map_erase(global_alloc_contexts, global_platform_api->platform_this_thread_id());
	global_platform_api->platform_release_mutex(global_alloc_contexts_mutex);

	return 0;
}
