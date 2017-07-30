
threadpool make_threadpool(i32 num_threads_) { PROF

	return make_threadpool(CURRENT_ALLOC(), num_threads_);
}

threadpool make_threadpool(allocator* a, i32 num_threads_) { PROF

	threadpool ret;

	ret.num_threads = num_threads_ == 0 ? global_state->api->platform_get_num_cpus() : num_threads_;

	ret.alloc   = a;
	ret.catalog = make_map<job_id,platform_semaphore>(16, hash_u64);
	ret.threads = make_array<platform_thread>(ret.num_threads, a);
	ret.data    = make_array<worker_data>(ret.num_threads, a);
	ret.jobs    = make_queue<job>(16, a);
	
	global_state->api->platform_create_mutex(&ret.queue_mutex, false);
	global_state->api->platform_create_semaphore(&ret.jobs_semaphore, 0, ret.num_threads);

	return ret;
}

void destroy_threadpool(threadpool* tp) { PROF

	threadpool_stop_all(tp);

	destroy_map(&tp->catalog);
	destroy_array(&tp->threads);
	destroy_array(&tp->data);
	destroy_queue(&tp->jobs);

	global_state->api->platform_destroy_mutex(&tp->queue_mutex);
	global_state->api->platform_destroy_semaphore(&tp->jobs_semaphore);
}

void threadpool_wait_job(threadpool* tp, job_id id) { PROF

	platform_semaphore* sem = map_get(&tp->catalog, id);
	global_state->api->platform_wait_semaphore(sem, -1);

	global_state->api->platform_aquire_mutex(&tp->catalog_mutex, -1);
	global_state->api->platform_destroy_semaphore(sem);
	map_erase(&tp->catalog, id);
	global_state->api->platform_release_mutex(&tp->catalog_mutex);
}

job_id threadpool_queue_job(threadpool* tp, void (*proc)(void*), void* data) { PROF


	job j;
	j.proc = proc;
	j.data = data;

	return threadpool_queue_job(tp, j);
}

job_id threadpool_queue_job(threadpool* tp, job j) { PROF

	global_state->api->platform_aquire_mutex(&tp->queue_mutex, -1);
	
	j.id = tp->next_job_id++;
	queue_push(&tp->jobs, j);

	global_state->api->platform_release_mutex(&tp->queue_mutex);

	platform_semaphore jid_sem;
	global_state->api->platform_create_semaphore(&jid_sem, 0, 1);

	// is this a good way to structure this? doesn't look like it
	global_state->api->platform_aquire_mutex(&tp->catalog_mutex, -1);
	map_insert(&tp->catalog, j.id, jid_sem);
	global_state->api->platform_release_mutex(&tp->catalog_mutex);

	global_state->api->platform_signal_semaphore(&tp->jobs_semaphore, 1);

	return j.id;
}

void threadpool_stop_all(threadpool* tp) { PROF

	if(tp->running) {
	
		for(i32 i = 0; i < tp->num_threads; i++) {

			array_get(&tp->data, i)->running = false;
		}

		global_state->api->platform_signal_semaphore(&tp->jobs_semaphore, tp->num_threads);

		for(i32 i = 0; i < tp->num_threads; i++) {

			global_state->api->platform_join_thread(array_get(&tp->threads, i), -1);
			global_state->api->platform_destroy_thread(array_get(&tp->threads, i));
		}

		tp->running = false;
	}
} 

void threadpool_start_all(threadpool* tp) { PROF

	if(!tp->running) {
	
		FORARR(tp->data,

			it->job_queue 	 	= &tp->jobs;
			it->queue_mutex 	= &tp->queue_mutex;
			it->jobs_semaphore 	= &tp->jobs_semaphore;
			it->running 		= true;
			it->alloc  			= tp->alloc;
			it->catalog 		= &tp->catalog;
			it->catalog_mutex 	= &tp->catalog_mutex;

			global_state->api->platform_create_thread(array_get(&tp->threads, __i), &worker, it, false);
		)

		tp->running = true;
	}
}

i32 worker(void* data_) { PROF

	worker_data* data = (worker_data*)data_;

	begin_thread(string_literal("worker %"), &global_state->suppressed_platform_allocator, (u32)global_state->api->platform_this_thread_id().id);
	LOG_DEBUG("Starting worker thread");

	while(data->running) {

		job current_job;

		global_state->api->platform_aquire_mutex(data->queue_mutex, -1);
		if(!queue_empty(data->job_queue)) {
			current_job = queue_pop(data->job_queue);
		}
		global_state->api->platform_release_mutex(data->queue_mutex);

		if(current_job.proc) {
			(*current_job.proc)(current_job.data);

			global_state->api->platform_aquire_mutex(data->catalog_mutex, -1);
			global_state->api->platform_signal_semaphore(map_get(data->catalog, current_job.id), 1);
			global_state->api->platform_release_mutex(data->catalog_mutex);
		}

		global_state->api->platform_wait_semaphore(data->jobs_semaphore, -1);
	}

	LOG_DEBUG("Ending worker thread");
	end_thread();

	return 0;
}
