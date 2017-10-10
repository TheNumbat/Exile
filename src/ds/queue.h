
#pragma once

// queue foreach
#define FORQ(q,code) {if((q).start != UINT32_MAX){u32 __i = (q).start; for(auto it = (q).memory + (q).start; __i != (q).end; ++__i %= (q).capacity, it = &(q).memory[__i]) {code}}}
#define FORQ2(q,code) {if((q).start != UINT32_MAX){u32 __i2 = (q).start; for(auto it2 = (q).memory + (q).start; __i2 != (q).end; ++__i2 %= (q).capacity, it2 = &(q).memory[__i2]) {code}}}

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
	T* push_overwrite(T value);

	T pop();
	bool try_pop(T* out);
	u32 len();

	T* get(u32 idx);

	T* front();
	T* back();
	bool empty();
};

// concurrent queue (reuses queue stuff)
template<typename T>
struct con_queue { // no inheritance LUL
	
	T* memory 		 = null;
	u32 start 		 = UINT32_MAX, end = 0;
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