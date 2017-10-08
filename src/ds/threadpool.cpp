
bool operator>(const job& l, const job& r) {
	return (u32)l.priority > (u32)r.priority;
}

threadpool threadpool::make(i32 num_threads_) { PROF

	return make(CURRENT_ALLOC(), num_threads_);
}

threadpool threadpool::make(allocator* a, i32 num_threads_) { PROF

	threadpool ret;

	ret.num_threads = num_threads_ == 0 ? global_api->platform_get_num_cpus() : num_threads_;

	ret.alloc   = a;
	ret.running = map<job_id,platform_semaphore>::make(16, FPTR(hash_u64));
	ret.threads = array<platform_thread>::make(ret.num_threads, a);
	ret.jobs    = con_heap<job>::make(16, a);
	ret.worker_data = array<worker_param>::make(ret.num_threads, a);
	
	global_api->platform_create_mutex(&ret.running_mutex, false);
	CHECKED(platform_create_semaphore, &ret.jobs_semaphore, 0, ret.num_threads);

	return ret;
}

void threadpool::destroy() { PROF

	stop_all();

	running.destroy();
	threads.destroy();
	worker_data.destroy();
	jobs.destroy();

	CHECKED(platform_destroy_semaphore, &jobs_semaphore);
}

void threadpool::wait_job(job_id id) { PROF

#ifdef NO_CONCURRENT_JOBS
	return;
#else
	platform_semaphore* sem = running.try_get(id);
	if(sem) {
		global_api->platform_wait_semaphore(sem, -1);
	}
#endif
}

job_id threadpool::queue_job(job_work work, void* data, job_priority priority) { PROF

	job j;
	j.priority = priority;
	j.work = work;
	j.data = data;

	return queue_job(j);
}

job_id threadpool::queue_job(job j) { PROF

#ifdef NO_CONCURRENT_JOBS
	j.work(j.data);
	return j.id;
#else
	j.id = next_job_id++;
	jobs.push(j);

	platform_semaphore jid_sem;
	CHECKED(platform_create_semaphore, &jid_sem, 0, INT_MAX);

	// is this a good way to structure this? doesn't look like it
	global_api->platform_aquire_mutex(&running_mutex);
	running.insert(j.id, jid_sem);
	global_api->platform_release_mutex(&running_mutex);

	CHECKED(platform_signal_semaphore, &jobs_semaphore, 1);

	return j.id;
#endif
}

void threadpool::stop_all() { PROF

	if(online) {
	
		for(i32 i = 0; i < num_threads; i++) {

			worker_data.get(i)->online = false;
		}

		CHECKED(platform_signal_semaphore, &jobs_semaphore, num_threads);

		for(i32 i = 0; i < num_threads; i++) {

			global_api->platform_join_thread(threads.get(i), -1);
			CHECKED(platform_destroy_thread, threads.get(i));
		}

		online = false;
	}
} 

void threadpool::start_all() { PROF

	if(!online) {
	
		FORARR(worker_data,

			it->job_queue 	 	= &jobs;
			it->jobs_semaphore 	= &jobs_semaphore;
			it->online 			= true;
			it->alloc  			= alloc;
			it->running 		= &running;
			it->running_mutex 	= &running_mutex;

			CHECKED(platform_create_thread, threads.get(__i), &worker, it, false);
		)

		online = true;
	}
}

i32 worker(void* data_) { 

	worker_param* data = (worker_param*)data_;

	begin_thread(np_string_literal("worker %"), data->alloc, (u32)global_api->platform_this_thread_id().id);
	LOG_DEBUG("Starting worker thread");

	while(data->online) {

		job current_job;

		if(data->job_queue->try_pop(&current_job)) {

			if(current_job.work) {
				job_callback callback = (*current_job.work)(current_job.data);

				platform_event a;
				a.type 			 = platform_event_type::async;
				a.async.type 	 = platform_async_type::user;
				a.async.user_id  = current_job.id;
				a.async.callback = callback;
				global_api->platform_queue_event(a);

				global_api->platform_aquire_mutex(data->running_mutex);
				platform_semaphore* sem = data->running->get(current_job.id);
				CHECKED(platform_signal_semaphore, sem, INT_MAX);
				CHECKED(platform_destroy_semaphore, sem);
				data->running->erase(current_job.id);
				global_api->platform_release_mutex(data->running_mutex);
			}
		}

		global_dbg->collate();
		global_api->platform_wait_semaphore(data->jobs_semaphore, -1);
	}

	LOG_DEBUG("Ending worker thread");
	end_thread();

	return 0;
}
