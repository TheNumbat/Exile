
#pragma once

#include "ds/heap.h"
#include "ds/array.h"
#include "util/threadstate.h"

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
	i32 priority_class  = 0;
	f32 priority 		= 0.0f;
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
	
	template<typename T> void queue_job(future<T>* fut, job_work<T> work, void* data = null, f32 prirority = 0.0f, i32 priority_class = 0, _FPTR* cancel = null);
	void queue_job(job_work<void> work, void* data = null, f32 prirority = 0.0f, i32 priority_class = 0, _FPTR* cancel = null);
	
	void stop_all();
	void start_all();

	void renew_priorities(f32 (*eval)(super_job*,void*), void* param);
};

i32 worker(void* data_);


template<typename E>
void atomic_enum<E>::set(E val) {

	global_api->atomic_exchange(&value, (u64)val);
}

template<typename E>
E atomic_enum<E>::get() {

	return (E)value;
}

template<typename T>
future<T> future<T>::make() {

	future<T> ret;

	global_api->create_mutex(&ret.mut, false);
	global_api->create_semaphore(&ret.sem, 0, INT_MAX);

	return ret;
}

template<typename T>
void future<T>::destroy() {

	global_api->destroy_mutex(&mut);
	global_api->destroy_semaphore(&sem);
}

template<typename T>
T future<T>::wait() {

	global_api->wait_semaphore(&sem, -1);
	return val;
}

template<typename T>
void future<T>::set(T v) {

	global_api->aquire_mutex(&mut);
	val = v;
	global_api->release_mutex(&mut);
	CHECKED(signal_semaphore, &sem, 1);
}

template<>
void heap<super_job*,gt>::renew(f32 (*eval)(super_job*, void*), void* param) { 

	heap<super_job*,gt> h = heap<super_job*,gt>::make(capacity);

	FORHEAP_LINEAR(it, *this) {

		super_job* j = *it;

		j->priority = eval(j, param);

		if(j->priority > -FLT_MAX) {
			h.push(j);
		} else {
			
			if(j->cancel)
				j->cancel(j->data);

			// NOTE(max): only works because the elements are allocated with the same allocator passed to the heap (in threadpool)
			PUSH_ALLOC(alloc) {
				free(j, j->my_size);
			} POP_ALLOC();
		}
	}
   
	_memcpy(h.memory, memory, h.size * sizeof(super_job*));
	size = h.size;
	h.destroy();
}

template<typename T>
void threadpool::queue_job(future<T>* fut, job_work<T> work, void* data, f32 priority, i32 priority_class, _FPTR* cancel) { 

	PUSH_ALLOC(alloc);

	job<T>* j = NEW(job<T>);
	j->priority = priority;
	j->priority_class = priority_class;
	j->future = fut;
	j->work = work;
	j->data = data;
	j->cancel.set(cancel);

#ifdef NO_CONCURRENT_JOBS
	j->do_work();
	free(j, j->my_size);
	POP_ALLOC();
#else

	jobs.push(j);

	CHECKED(signal_semaphore, &jobs_semaphore, 1);

	POP_ALLOC();
#endif
}
