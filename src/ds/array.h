
#pragma once

template<typename T>
struct array {
	T* memory 		 = NULL;
	u32 capacity 	 = 0;
	allocator* alloc = NULL;
};

template<typename T> array<T> make_array(u32 capacity, allocator* a);
template<typename T> array<T> make_array(u32 capacity);
template<typename T> array<T> make_array_memory(u32 capacity, void* memory);
template<typename T> void destroy_array(array<T>* a);

template<typename T> u32 array_len(array<T>* a);
template<typename T> T* array_get(array<T>* a, u32 idx);
