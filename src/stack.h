
#pragma once

#include "vector.h"

template<typename T>
using stack = vector<T>;

template<typename T>
void destroy_stack(stack<T>* s) {

	destroy_vector<T>(s);
}

template<typename T>
stack<T> make_stack(i32 capacity, allocator* a) {

	stack<T> ret;

	ret = make_vector<T>(capacity, a);
	
	return ret;
}

template<typename T>
stack<T> make_stack(i32 capacity = 0) {

	stack<T> ret;

	ret = make_vector(capacity);

	return ret;
}

template<typename T>
void stack_push(stack<T>* s, T value) {

	vector_push(s, value);
}

template<typename T>
T stack_pop(stack<T>* s) {

	if(s->size > 0) {
		
		T top = stack_top(s);

		vector_pop(s);

		return top;	
	}

	// TODO(max): errors
	T ret;
	return ret;
}

template<typename T>
T stack_top(stack<T>* s) {

	if(s->size > 0) {

		return vector_back(s);

	}

	// TODO(max): errors
	T ret;
	return ret;
}