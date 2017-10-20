
#pragma once

// array foreach
#define FORARR(it,a) u32 __##it = 0; for(auto it = (a).memory; it != (a).memory + (a).capacity; __##it++, it++)

template<typename T>
struct array {
	T* memory 		 = null;
	u32 capacity 	 = 0;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static array<T> make(u32 capacity);
	static array<T> make(u32 capacity, allocator* a);
	static array<T> make_copy(array<T>* src, allocator* a);
	static array<T> make_memory(u32 capacity, void* memory);

	void destroy();
	
	u32 len();
	T* get(u32 idx);
};
