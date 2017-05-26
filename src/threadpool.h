
#pragma once;

struct job {
	i32 (*proc)(void*) = NULL;
	void* data 		   = NULL;
};

struct worker_data {
	queue<job>* job_queue 			   = NULL;
	platform_mutex* queue_mutex 	   = NULL;
	platform_semaphore* jobs_semaphore = NULL;
	allocator* alloc 				   = NULL;
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

