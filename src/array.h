
#pragma once

#include "vector.h"

template<typename T>
struct array {
	T* memory 		 = NULL;
	i32 capacity 	 = 0;
	allocator* alloc = NULL;
};

template<typename T> 
i32 array_len(array<T>* a) {
	return a->capacity;
}

template<typename T>
void destroy_array(array<T>* a) {

	if(a->memory) {

		a->alloc->free(a->memory, a->alloc);
	}

	a->memory = NULL;
	a->capacity = 0;
}

template<typename T>
array<T> make_array(i32 capacity, allocator* a) {

	array<T> ret;

	a->alloc = a;
	a->capacity = capacity;

	if(capacity > 0) {

		a->memory = a->alloc->allocate(capacity * sizeof(T), a->alloc);
	}
	
	return ret;
}

template<typename T>
array<T> make_array(i32 capacity) {

	array<T> ret;

	ret = make_array(capacity, CURRENT_ALLOC());

	return ret;
}

template<typename T>
array<T> make_array_memory(i32 capacity, void* memory) {

	array<T> ret;

	a->alloc = a;
	a->capacity = capacity;
	a->memory = memory;

	return ret;
}

// operator[] but not a member
template<typename T>
T& get(array<T>* a, i32 idx) {

	if(a->memory && idx >= 0 && idx < a->capacity) {
		return a->memory[idx];
	}

	// TODO(max): error
	T ret = {};
	return ret;
}