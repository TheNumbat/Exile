
#pragma once

template<typename T>
struct queue {
	T* memory 		 = null;
	u32 start 		 = 0, end = 0;
	u32 capacity 	 = 0;
	allocator* alloc = null;
};

template<typename T> queue<T> make_queue(u32 capacity, allocator* a);
template<typename T> queue<T> make_queue(u32 capacity = 0);
template<typename T> void destroy_queue(queue<T>* q);
template<typename T> void queue_grow(queue<T>* q);

template<typename T> T* queue_push(queue<T>* q, T value);
template<typename T> T queue_pop(queue<T>* q);
template<typename T> bool queue_try_pop(queue<T>* q, T* out);
template<typename T> u32 queue_len(queue<T>* q);

template<typename T> T* queue_front(queue<T>* q);
template<typename T> T* queue_back(queue<T>* q);
template<typename T> bool queue_empty(queue<T>* q);

// TODO(max): Lock-free dequeue for threadpool

// concurrent queue (reuses queue stuff)
template<typename T>
struct con_queue { // no inheritance LUL
	T* memory 		 = null;
	u32 start 		 = 0, end = 0;
	u32 capacity 	 = 0;
	allocator* alloc = null;

	platform_mutex 		mut;
	platform_semaphore 	sem;
};

template<typename T> con_queue<T> make_con_queue(u32 capacity, allocator* a);
template<typename T> con_queue<T> make_con_queue(u32 capacity = 0);
template<typename T> void destroy_con_queue(con_queue<T>* q);

template<typename T> T* queue_push(con_queue<T>* q, T value);
template<typename T> T queue_wait_pop(con_queue<T>* q);
template<typename T> bool queue_try_pop(con_queue<T>* q, T* out);
