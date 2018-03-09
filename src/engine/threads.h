
#pragma once

template<typename E>
struct atomic_enum {

	void set(E val);
	E get();

private:
	u64 value = (u64)E::none;
	friend void make_meta_info();
};

// TODO(max): work-stealing thread local queues to minimize locking

template<typename T>
struct future {
private:
	T val;
	platform_semaphore sem;
	platform_mutex mut;
	friend void make_meta_info();

public:
	static future make(); 
	void destroy();

	T wait();
	void set(T val);
};

template<>
struct NOREFLECT future<void> {};

template<typename T>
using job_work = T(*)(void*);

struct super_job {
	float priority 		= 0.0f;
	void* data 	  		= null;
	u64 my_size			= 0;
	func_ptr<void,void*> cancel;
	virtual void do_work() = 0; // NOTE(max): pretty sure this is the only way to make this work...and it breaks hot reloading.
								//  		  AS A RESULT we need to make sure the threadpool queue is empty before reloading
};

bool gt(super_job* l, super_job* r);

template<typename T>
struct job : super_job {
	job() { my_size = sizeof(job<T>); };
	future<T>* future = null;
	job_work<T> work  = null;
	void do_work() { future->set(work(data)); }
};

template<>
struct NOREFLECT job<void> : super_job {
	job() { my_size = sizeof(job<void>); };
	job_work<void> work;
	void do_work() { work(data); }
};

struct worker_param {
	locking_heap<super_job*,gt>* job_queue 	= null;
	platform_semaphore* jobs_semaphore 		= null;
	allocator* alloc 				   		= null;
	bool online			   	       			= false;
};

struct threadpool {
	i32 num_threads 	= 0;
	bool online    		= false;

	locking_heap<super_job*,gt> jobs;		

	array<platform_thread> 	threads;
	array<worker_param> 	worker_data;
	
	platform_semaphore	   jobs_semaphore;
	allocator* 			   alloc;

///////////////////////////////////////////////////////////////////////////////

	static threadpool make(i32 num_threads_ = 0);
	static threadpool make(allocator* a, i32 num_threads_ = 0);
	void destroy();
	
	template<typename T> void queue_job(future<T>* fut, job_work<T> work, void* data = null, float prirority = 0.0f, _FPTR* cancel = null);
	void queue_job(job_work<void> work, void* data = null, float prirority = 0.0f, _FPTR* cancel = null);
	
	void stop_all();
	void start_all();

	void renew_priorities(float (*eval)(super_job*,void*), void* param);
};

i32 worker(void* data_);
