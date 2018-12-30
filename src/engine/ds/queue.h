
#pragma once

// queue foreach
#define FORQ_BEGIN(it,q) 	if((q).start != UINT32_MAX){u32 __##it = (q).start; do {auto it = &(q).memory[__##it];
#define FORQ_END(it,q) 		++__##it %= (q).capacity;} while(__##it != (q).end);}

template<typename T>
struct queue {
	T* memory 		 = null;
	u32 start 		 = UINT32_MAX, end = 0;
	u32 capacity 	 = 0;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static queue<T> make(u32 capacity, allocator* a);
	static queue<T> make(u32 capacity = 0);
	void destroy();
	
	void clear();
	void grow();

	T* push(T value);
	T push_overwrite(T value);

	T pop();
	bool try_pop(T* out);
	u32 len();
	bool full();

	T* get(u32 idx);

	T* front();
	T* back();
	bool empty();
};

// concurrent queue 
template<typename T>
struct locking_queue : public queue<T> { 
	
	platform_mutex 		mut;
	platform_semaphore 	sem;

///////////////////////////////////////////////////////////////////////////////

	static locking_queue<T> make(u32 capacity, allocator* a);
	static locking_queue<T> make(u32 capacity = 0);
	void destroy();

	T* push(T value);
	T wait_pop();
	bool try_pop(T* out);
	bool empty();
};
