
#pragma once

template<typename T>
struct queue {
	T* memory 		 = null;
	u32 start 		 = 0, end = 0;
	u32 capacity 	 = 0;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static queue<T> make(u32 capacity, allocator* a);
	static queue<T> make(u32 capacity = 0);
	void destroy();
	
	void grow();
	T* push(T value);
	T pop();
	bool try_pop(T* out);
	u32 len();

	T* front();
	T* back();
	bool empty();
};

// TODO(max): Lock-free dequeue for threadpool

// concurrent queue (reuses queue stuff)
template<typename T>
struct con_queue { // no inheritance LUL
	T* memory 		 = null;
	u32 start 		 = 0, end = 0;
	u32 capacity 	 = 0;
	allocator* alloc = null;

	platform_mutex 		mut;
	platform_semaphore 	sem;

///////////////////////////////////////////////////////////////////////////////

	static con_queue<T> make(u32 capacity, allocator* a);
	static con_queue<T> make(u32 capacity = 0);
	void destroy();

	T* push(T value);
	T wait_pop();
	bool try_pop(T* out);
};