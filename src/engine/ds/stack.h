
#pragma once

#include "vector.h"

template<typename T>
struct stack {
	vector<T> contents;

///////////////////////////////////////////////////////////////////////////////

	static stack<T> make(u32 capacity, allocator* a);
	static stack<T> make(u32 capacity = 0);
	static stack<T> make_copy(stack<T> src);
	static stack<T> make_copy(stack<T> src, allocator* a);
	static stack<T> make_copy_trim(stack<T> src, allocator* a);
	void destroy();
	void clear();

	void push(T value);
	T pop();
	bool try_pop(T* out);

	T* top();
	bool empty();
};

template<typename T>
struct locking_stack : public stack<T> {

	platform_mutex mut;
	platform_semaphore sem;

///////////////////////////////////////////////////////////////////////////////

	static locking_stack<T> make(u32 capacity, allocator* a);
	static locking_stack<T> make(u32 capacity = 0);
	void destroy();

	T* push(T value);
	T wait_pop();
	bool try_pop(T* out);
};
