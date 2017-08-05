
threadpool make_threadpool(i32 num_threads_) { PROF

	return make_threadpool(CURRENT_ALLOC(), num_threads_);
}

threadpool make_threadpool(allocator* a, i32 num_threads_) { PROF

	threadpool ret;

	ret.num_threads = num_threads_ == 0 ? global_api->platform_get_num_cpus() : num_threads_;

	ret.alloc   = a;
	ret.running = make_map<job_id,platform_semaphore>(16, hash_u64);
	ret.threads = array<platform_thread>::make(ret.num_threads, a);
	ret.data    = array<worker_data>::make(ret.num_threads, a);
	ret.jobs    = make_con_queue<job>(16, a);
	
	global_api->platform_create_semaphore(&ret.jobs_semaphore, 0, ret.num_threads);

	return ret;
}

void destroy_threadpool(threadpool* tp) { PROF

	threadpool_stop_all(tp);

	destroy_map(&tp->running);
	tp->threads.destroy();
	tp->data.destroy();
	destroy_con_queue(&tp->jobs);

	global_api->platform_destroy_semaphore(&tp->jobs_semaphore);
}

void threadpool_wait_job(threadpool* tp, job_id id) { PROF

	platform_semaphore* sem = map_try_get(&tp->running, id);
	if(sem) {
		global_api->platform_wait_semaphore(sem, -1);
	}
}

job_id threadpool_queue_job(threadpool* tp, job_work work, void* data) { PROF

	job j;
	j.work = work;
	j.data = data;

	return threadpool_queue_job(tp, j);
}

job_id threadpool_queue_job(threadpool* tp, job j) { PROF

	j.id = tp->next_job_id++;
	queue_push(&tp->jobs, j);

	platform_semaphore jid_sem;
	global_api->platform_create_semaphore(&jid_sem, 0, INT_MAX);

	// is this a good way to structure this? doesn't look like it
	global_api->platform_aquire_mutex(&tp->running_mutex, -1);
	map_insert(&tp->running, j.id, jid_sem);
	global_api->platform_release_mutex(&tp->running_mutex);

	global_api->platform_signal_semaphore(&tp->jobs_semaphore, 1);

	return j.id;
}

void threadpool_stop_all(threadpool* tp) { PROF

	if(tp->online) {
	
		for(i32 i = 0; i < tp->num_threads; i++) {

			tp->data.get(i)->online = false;
		}

		global_api->platform_signal_semaphore(&tp->jobs_semaphore, tp->num_threads);

		for(i32 i = 0; i < tp->num_threads; i++) {

			global_api->platform_join_thread(tp->threads.get(i), -1);
			global_api->platform_destroy_thread(tp->threads.get(i));
		}

		tp->online = false;
	}
} 

void threadpool_start_all(threadpool* tp) { PROF

	if(!tp->online) {
	
		FORARR(tp->data,

			it->job_queue 	 	= &tp->jobs;
			it->jobs_semaphore 	= &tp->jobs_semaphore;
			it->online 			= true;
			it->alloc  			= tp->alloc;
			it->running 		= &tp->running;
			it->running_mutex 	= &tp->running_mutex;

			global_api->platform_create_thread(tp->threads.get(__i), &worker, it, false);
		)

		tp->online = true;
	}
}

i32 worker(void* data_) { PROF

	worker_data* data = (worker_data*)data_;

	begin_thread(string_literal("worker %"), data->alloc, (u32)global_api->platform_this_thread_id().id);
	LOG_DEBUG("Starting worker thread");

	while(data->online) {

		job current_job;

		if(queue_try_pop(data->job_queue, &current_job)) {

			if(current_job.work) {
				job_callback callback = (*current_job.work)(current_job.data);

				platform_event a;
				a.type 			 = platform_event_type::async;
				a.async.type 	 = platform_async_type::user;
				a.async.user_id  = current_job.id;
				a.async.callback = callback;
				global_api->platform_queue_event(a);

				global_api->platform_aquire_mutex(data->running_mutex, -1);
				platform_semaphore* sem = map_get(data->running, current_job.id);
				global_api->platform_signal_semaphore(sem, INT_MAX);
				global_api->platform_destroy_semaphore(sem);
				map_erase(data->running, current_job.id);
				global_api->platform_release_mutex(data->running_mutex);
			}
		}

		global_api->platform_wait_semaphore(data->jobs_semaphore, -1);
	}

	LOG_DEBUG("Ending worker thread");
	end_thread();

	return 0;
}
