
#pragma once

template<typename T>
struct queue {vector<T> contents;};

// TODO(max): circular queue instead of vector_pop_front

template<typename T> queue<T> make_queue(u32 capacity, allocator* a);
template<typename T> queue<T> make_queue(u32 capacity = 0);
template<typename T> void destroy_queue(queue<T>* q);

template<typename T> T* queue_push(queue<T>* q, T value);
template<typename T> T  queue_pop(queue<T>* q);

template<typename T> T* queue_front(queue<T>* q);
template<typename T> T* queue_back(queue<T>* q);
template<typename T> bool queue_empty(queue<T>* q);
