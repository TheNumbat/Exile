
bool operator>(const job& l, const job& r) { PROF
	return (u32)l.priority > (u32)r.priority;
}

threadpool threadpool::make(i32 num_threads_) { PROF

	return make(CURRENT_ALLOC(), num_threads_);
}

threadpool threadpool::make(allocator* a, i32 num_threads_) { PROF

	threadpool ret;

	ret.num_threads = num_threads_ == 0 ? global_api->get_num_cpus() : num_threads_;

	ret.alloc   = a;
	ret.running = map<job_id,platform_semaphore>::make(16);
	ret.threads = array<platform_thread>::make(ret.num_threads, a);
	ret.jobs    = locking_heap<job>::make(16, a);
	ret.worker_data = array<worker_param>::make(ret.num_threads, a);
	
	global_api->create_mutex(&ret.running_mutex, false);
	CHECKED(create_semaphore, &ret.jobs_semaphore, 0, ret.num_threads);

	return ret;
}

void threadpool::destroy() { PROF

	stop_all();

	running.destroy();
	threads.destroy();
	worker_data.destroy();
	jobs.destroy();

	CHECKED(destroy_semaphore, &jobs_semaphore);
}

void threadpool::wait_job(job_id id) { PROF

#ifdef NO_CONCURRENT_JOBS
	return;
#else
	global_api->aquire_mutex(&running_mutex);
	platform_semaphore* sem = running.try_get(id);
	global_api->release_mutex(&running_mutex);

	// TODO(max): evil - this might explode if you insert a new job and it moves the semaphore this is waiting on
	// 			  We can't just keep the mutex until we're doing waiting because that could deadlock when acquiring
	// 			  the mutex to signal and remove the job in a worker thread
	// 			  This is fine if only one thread is ever adding or waiting on jobs...which isn't the case
	// 			  **Should whoever adds the job have to provide a semaphore to wait on?**
	if(sem) {
		global_api->wait_semaphore(sem, -1);
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
	CHECKED(create_semaphore, &jid_sem, 0, INT_MAX);

	// is this a good way to structure this? doesn't look like it
	global_api->aquire_mutex(&running_mutex);
	running.insert(j.id, jid_sem);
	global_api->release_mutex(&running_mutex);

	CHECKED(signal_semaphore, &jobs_semaphore, 1);

	return j.id;
#endif
}

void threadpool::stop_all() { PROF

	if(online) {
	
		for(i32 i = 0; i < num_threads; i++) {

			worker_data.get(i)->online = false;
		}

		CHECKED(signal_semaphore, &jobs_semaphore, num_threads);

		for(i32 i = 0; i < num_threads; i++) {

			global_api->join_thread(threads.get(i), -1);
			CHECKED(destroy_thread, threads.get(i));
		}

		online = false;
	}
} 

void threadpool::start_all() { PROF

	if(!online) {
	
		FORARR(it, worker_data) {

			it->job_queue 	 	= &jobs;
			it->jobs_semaphore 	= &jobs_semaphore;
			it->online 			= true;
			it->alloc  			= alloc;
			it->running 		= &running;
			it->running_mutex 	= &running_mutex;

			CHECKED(create_thread, threads.get(__it), &worker, it, false);
		}

		online = true;
	}
}

i32 worker(void* data_) { 

	worker_param* data = (worker_param*)data_;

	begin_thread("worker %"_, data->alloc, global_api->this_thread_id());
	this_thread_data.profiling = false;
	global_dbg->register_thread(10, 8192);
	
	LOG_DEBUG("Starting worker thread");

	while(data->online) {

		job current_job;

		if(data->job_queue->try_pop(&current_job)) {

			if(current_job.work) {

				BEGIN_FRAME();

				job_callback callback;
				PUSH_PROFILE(true) {
					callback = (*current_job.work)(current_job.data);
				} POP_PROFILE();

				END_FRAME();

				platform_event a;
				a.type 			 = platform_event_type::async;
				a.async.type 	 = platform_async_type::user;
				a.async.user_id  = current_job.id;
				a.async.callback = callback;
				global_api->queue_event(a);

				global_api->aquire_mutex(data->running_mutex);
				platform_semaphore* sem = data->running->get(current_job.id);
				CHECKED(signal_semaphore, sem, INT_MAX);
				CHECKED(destroy_semaphore, sem);
				data->running->erase(current_job.id);
				global_api->release_mutex(data->running_mutex);
			}
		}

		global_dbg->collate();
		global_api->wait_semaphore(data->jobs_semaphore, -1);
	}

	LOG_DEBUG("Ending worker thread");
	end_thread();

	return 0;
}
