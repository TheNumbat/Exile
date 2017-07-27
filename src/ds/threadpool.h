
#pragma once

// TODO(max): work-stealing thread local queues to minimize locking

typedef u64 job_id;

struct job {
	job_id id 		   = 0;
	void (*proc)(void*) = NULL;
	void* data 		   = NULL;
};

struct worker_data {
	queue<job>* job_queue 			   		= NULL;
	map<job_id,platform_semaphore>* catalog = NULL;
	platform_mutex* queue_mutex 	   		= NULL;
	platform_mutex* catalog_mutex 	   		= NULL;
	platform_semaphore* jobs_semaphore 		= NULL;
	allocator* alloc 				   		= NULL;
	bool running			   	       		= false;
};

struct threadpool {
	i32 num_threads 	= 0;
	job_id next_job_id	= 1;
	bool running    	= false;

	queue<job> jobs;			 			// TODO(max): priority queue
	map<job_id,platform_semaphore> catalog; // return values, INT_MIN if running

	array<platform_thread> threads;
	array<worker_data> 	   data;
	
	platform_mutex		   queue_mutex, catalog_mutex; 
	platform_semaphore	   jobs_semaphore;
	allocator* 			   alloc;
};

threadpool make_threadpool(i32 num_threads_ = 0);
threadpool make_threadpool(allocator* a, i32 num_threads_ = 0);
void destroy_threadpool(threadpool* tp);

job_id threadpool_queue_job(threadpool* tp, void (*proc)(void*), void* data);
job_id threadpool_queue_job(threadpool* tp, job j);
void threadpool_wait_job(threadpool* tp, job_id id);

void threadpool_stop_all(threadpool* tp);
void threadpool_start_all(threadpool* tp);

i32 worker(void* data_);

