
#pragma once

// TODO(max): work-stealing thread local queues to minimize locking

typedef void (*job_callback)();
typedef job_callback (*job_work)(void*);

struct job {
	job_id id	  = 0;
	job_work work = null;
	void* data 	  = null;
};

struct worker_data {
	queue<job>* job_queue 			   		= null;
	map<job_id,platform_semaphore>* running = null;
	platform_mutex* queue_mutex 	   		= null;
	platform_mutex* running_mutex 	   		= null;
	platform_semaphore* jobs_semaphore 		= null;
	allocator* alloc 				   		= null;
	bool online			   	       			= false;
};

struct threadpool {
	i32 num_threads 	= 0;
	job_id next_job_id	= 1;
	bool online    		= false;

	queue<job> jobs;			 			// TODO(max): priority queue
	map<job_id,platform_semaphore> running;

	array<platform_thread> threads;
	array<worker_data> 	   data;
	
	platform_mutex		   queue_mutex, running_mutex; 
	platform_semaphore	   jobs_semaphore;
	allocator* 			   alloc;
};

threadpool make_threadpool(i32 num_threads_ = 0);
threadpool make_threadpool(allocator* a, i32 num_threads_ = 0);
void destroy_threadpool(threadpool* tp);

job_id threadpool_queue_job(threadpool* tp, job_work work, void* data);
job_id threadpool_queue_job(threadpool* tp, job j);
void threadpool_wait_job(threadpool* tp, job_id id);

void threadpool_stop_all(threadpool* tp);
void threadpool_start_all(threadpool* tp);

i32 worker(void* data_);

