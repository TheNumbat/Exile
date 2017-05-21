
#pragma once

#include "vector.h"

template<typename T>
struct array {vector<T> contents;};

template<typename T>
void destroy_array(array<T>* a) {

	destroy_vector<T>(&a->contents);
}

template<typename T>
array<T> make_array(i32 capacity, allocator* a) {

	array<T> ret;

	ret.contents = make_vector<T>(capacity, a);
	
	return ret;
}

template<typename T>
array<T> make_array(i32 capacity) {

	array<T> ret;

	ret.contents = make_vector<T>(capacity);

	return ret;
}

// operator[] but not a member
template<typename T>
T& get(array<T>* a, i32 idx) {

	if(a->contents.memory && idx >= 0 && idx < a->contents.capacity) {
		return a->contents.memory[idx];
	}

	// TODO(max): error
	T ret = {};
	return ret;
}