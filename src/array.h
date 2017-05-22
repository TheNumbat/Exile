
#pragma once

#include "vector.h"

template<typename T>
struct array {
	T* memory 		 = NULL;
	i32 capacity 	 = 0;
	allocator* alloc = NULL;
};


template<typename T> array<T> make_array(i32 capacity, allocator* a);
template<typename T> array<T> make_array(i32 capacity);
template<typename T> array<T> make_array_memory(i32 capacity, void* memory);
template<typename T> void destroy_array(array<T>* a);

template<typename T> i32 array_len(array<T>* a);
template<typename T> T& get(array<T>* a, i32 idx);


template<typename T> 
i32 array_len(array<T>* a) {
	return a->capacity;
}

template<typename T>
void destroy_array(array<T>* a) {

	if(a->alloc && a->memory) {

		a->alloc->free_(a->memory, a->alloc);
	}

	a->memory = NULL;
	a->capacity = 0;
}

template<typename T>
array<T> make_array(i32 capacity, allocator* a) {

	array<T> ret;

	ret.alloc = a;
	ret.capacity = capacity;

	if(capacity > 0) {

		ret.memory = (T*)ret.alloc->allocate_(capacity * sizeof(T), ret.alloc);
	}
	
	return ret;
}

template<typename T>
array<T> make_array(i32 capacity) {

	array<T> ret;

	ret = make_array<T>(capacity, CURRENT_ALLOC());

	return ret;
}

template<typename T>
array<T> make_array_memory(i32 capacity, void* memory) {

	array<T> ret;

	ret.capacity = capacity;
	ret.memory = (T*)memory;

	return ret;
}

// operator[] but not a member
template<typename T>
T& get(array<T>* a, i32 idx) {

	if(a->memory && idx >= 0 && idx < a->capacity) {
		return a->memory[idx];
	}

	// TODO(max): error
	assert(false);
	T ret = {};
	return ret;
}