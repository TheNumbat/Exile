
#pragma once

// TODO(max): work-stealing thread local queues to minimize locking

typedef void (*job_callback)();
typedef job_callback (*job_work)(void*);

enum class job_priority : u8 {
	normal = 0,
	high,
	asap
};

struct job {
	job_priority priority = job_priority::normal;
	job_id id	  = 0;
	job_work work = null;
	void* data 	  = null;
};

struct worker_param {
	locking_heap<job>* job_queue 			= null;
	map<job_id,platform_semaphore>* running = null;
	platform_mutex* running_mutex 	   		= null;
	platform_semaphore* jobs_semaphore 		= null;
	allocator* alloc 				   		= null;
	bool online			   	       			= false;
};

struct threadpool {
	i32 num_threads 	= 0;
	job_id next_job_id	= 1;
	bool online    		= false;

	locking_heap<job> jobs;		
	map<job_id,platform_semaphore> running;

	array<platform_thread> 	threads;
	array<worker_param> 	worker_data;
	
	platform_mutex		   running_mutex; 
	platform_semaphore	   jobs_semaphore;
	allocator* 			   alloc;

///////////////////////////////////////////////////////////////////////////////

	static threadpool make(i32 num_threads_ = 0);
	static threadpool make(allocator* a, i32 num_threads_ = 0);
	void destroy();
	
	job_id queue_job(job_work work, void* data, job_priority prirority = job_priority::normal);
	job_id queue_job(job j);
	void wait_job(job_id id);
	
	void stop_all();
	void start_all();
};

bool operator>(const job& l, const job& r);
i32 worker(void* data_);
