
#pragma once

#define FORHEAP_LINEAR(it,v) for(auto it = (v).memory; it != (v).memory + (v).size; it++)

template<typename T, bool(comp)(T,T) = gt>
struct heap {

	T* memory = null;
	u32 size = 0;
	u32 capacity = 0;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static heap<T, comp> make(u32 capacity = 8, allocator* alloc = null);
	void destroy();

	void clear();
	void grow();
	
	void push(T value);
	T pop();
	bool try_pop(T* out);
	
	bool empty();

	void reheap_up(u32 node);
	void reheap_down(u32 root = 0);
	void renew(float (*eval)(T,void*), void* param);
};

template<typename T, bool(comp)(T,T) = gt>
struct locking_heap : public heap<T, comp> {

	platform_mutex 		mut;
	platform_semaphore 	sem;

///////////////////////////////////////////////////////////////////////////////

	static locking_heap<T, comp> make(u32 capacity = 8, allocator* alloc = null);
	void destroy();

	void push(T value);
	T wait_pop();
	bool try_pop(T* out);
	void renew(float (*eval)(T,void*), void* param);
};

