
#pragma once;

#include "common.h"

struct job {
	i32 (*proc)(void*) = NULL;
	void* data 		   = NULL;
};

struct worker_data {
	queue<job>* job_queue 			   = NULL;
	platform_mutex* queue_mutex 	   = NULL;
	platform_semaphore* jobs_semaphore = NULL;
	volatile bool running			   = false;
};

struct threadpool {
	i32 num_threads = 0;
	bool running    = false;
	array<platform_thread> threads;
	array<worker_data> 	   data;
	queue<job>	   	 	   jobs; 		// TODO(max): make a concurrent queue
	platform_mutex		   queue_mutex; 
	platform_semaphore	   jobs_semaphore;
	allocator* 			   alloc = NULL;
};


threadpool make_threadpool(i32 num_threads_ = 0);
threadpool make_threadpool(allocator* a, i32 num_threads_ = 0);
void destroy_threadpool(threadpool* tp);

void threadpool_queue_job(threadpool* tp, i32 (*proc)(void*), void* data);
void threadpool_queue_job(threadpool* tp, job j);
void threadpool_stop_all(threadpool* tp);
void threadpool_start_all(threadpool* tp);

i32 worker(void* data_);

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
		cout << "Error creating mutex: " << err.error << endl;
	}
	err = global_platform_api->platform_create_semaphore(&ret.jobs_semaphore, 0, ret.num_threads);
	if(!err.good) {
		cout << "Error creating semaphore: " << err.error << endl;	
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
		cout << "error acquiring mutex: " << state.error.error << endl;
	}

	queue_push(&tp->jobs, j);

	platform_error err = global_platform_api->platform_release_mutex(&tp->queue_mutex);
	if(!err.good) {
		cout << "error releasing mutex: " << err.error << endl;
	}

	err = global_platform_api->platform_signal_semaphore(&tp->jobs_semaphore, 1);
	if(!err.good) {
		cout << "error signaling semaphore: " << err.error << endl;
	}
}

void threadpool_stop_all(threadpool* tp) {

	if(tp->running) {
	
		for(i32 i = 0; i < tp->num_threads; i++) {

			get(&tp->data, i).running = false;
		}

		global_platform_api->platform_signal_semaphore(&tp->jobs_semaphore, tp->num_threads);

		for(i32 i = 0; i < tp->num_threads; i++) {

			global_platform_api->platform_join_thread(&get(&tp->threads, i), -1);
			global_platform_api->platform_destroy_thread(&get(&tp->threads, i));
		}

		tp->running = false;
	}
} 

void threadpool_start_all(threadpool* tp) {

	if(!tp->running) {
	
		for(i32 i = 0; i < tp->num_threads; i++) {

			get(&tp->data, i).job_queue 	 = &tp->jobs;
			get(&tp->data, i).queue_mutex 	 = &tp->queue_mutex;
			get(&tp->data, i).jobs_semaphore = &tp->jobs_semaphore;
			get(&tp->data, i).running = true;

			global_platform_api->platform_create_thread(&get(&tp->threads, i), &worker, &get(&tp->data, i), false);
		}

		tp->running = true;
	}
}

i32 worker(void* data_) {

	worker_data* data = (worker_data*)data_;

	// TODO(max): errors

	cout << "starting thread" << endl;

	while(data->running) {
		job current_job;

		platform_mutex_state state = global_platform_api->platform_aquire_mutex(data->queue_mutex, -1);
		if(!state.error.good) {
			cout << "error acquiring mutex: " << state.error.error << endl;
		}

		if(!queue_empty(data->job_queue)) {
			current_job = queue_pop(data->job_queue);
		}

		platform_error err = global_platform_api->platform_release_mutex(data->queue_mutex);
		if(!err.good) {
			cout << "error releasing mutex: " << err.error << endl;
		}

		if(current_job.proc) {
			(*current_job.proc)(current_job.data);
		}

		//cout << "waiting" << endl;

		platform_semaphore_state state_ = global_platform_api->platform_wait_semaphore(data->jobs_semaphore, -1);
		if(!state_.error.good) {
			cout << "error waiting semaphore: " << state_.error.error << endl;
		}
	}

	cout << "done" << endl;

	return 0;
}
