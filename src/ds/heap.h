
#pragma once

template<typename T>
struct heap {

	T* memory = null;
	u32 size = 0;
	u32 capacity = 0;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static heap<T> make(u32 capacity = 8, allocator* alloc = null);
	void destroy();

	void clear();
	void grow();
	
	void push(T value);
	T pop();
	bool try_pop(T* out);
	
	bool empty();

	void reheap_up(u32 node);
	void reheap_down(u32 root = 0);
};
