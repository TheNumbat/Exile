
#pragma once

#include "vector.h"

template<typename T>
struct queue {vector<T> contents;};


template<typename T> queue<T> make_queue(i32 capacity, allocator* a);
template<typename T> queue<T> make_queue(i32 capacity = 0);
template<typename T> void destroy_queue(queue<T>* q);

template<typename T> void queue_push(queue<T>* q, T value);
template<typename T> T queue_pop(queue<T>* q);

template<typename T> T queue_front(queue<T>* q);
template<typename T> bool queue_empty(queue<T>* q);


template<typename T>
void destroy_queue(queue<T>* q) {

	destroy_vector(&q->contents);
}

template<typename T>
queue<T> make_queue(i32 capacity, allocator* a) {

	queue<T> ret;

	ret.contents = make_vector<T>(capacity, a);
	
	return ret;
}

template<typename T>
queue<T> make_queue(i32 capacity) {

	queue<T> ret;

	ret.contents = make_vector<T>(capacity);

	return ret;
}

template<typename T>
void queue_push(queue<T>* q, T value) {

	vector_push(&q->contents, value);
}

template<typename T>
T queue_pop(queue<T>* q) {

	if(q->contents.size > 0) {
		
		T top = queue_front(q);

		vector_pop_front(&q->contents);

		return top;	
	}

	// TODO(max): errors
	T ret = {};
	return ret;
}

template<typename T>
T queue_front(queue<T>* q) {

	if(q->contents.size > 0) {

		return vector_front(&q->contents);
	}

	// TODO(max): errors
	T ret = {};
	return ret;
}

template<typename T>
bool queue_empty(queue<T>* q) {
	return vector_empty(&q->contents);
}