
#pragma once

#include "heap.h"

template<typename T>
heap<T> heap<T>::make(u32 capacity, allocator* alloc) { 

	heap<T> ret;
	ret.capacity = capacity;
	
	if(!alloc) alloc = CURRENT_ALLOC();

	ret.alloc = alloc;

	PUSH_ALLOC(alloc) {
		ret.memory = NEWA(T, capacity);
	} POP_ALLOC();

	return ret;
}	

template<typename T>
void heap<T>::destroy() { 

	if(memory) {
		alloc->free_(memory, capacity * sizeof(T), alloc, CONTEXT); 
		memory = null;
	}

	size = capacity = 0;
}

template<typename T>
void heap<T>::clear() { 

	size = 0;
}

template<typename T>
void heap<T>::grow() { 

	memory = (T*)alloc->reallocate_(memory, capacity * sizeof(T), 2 * capacity * sizeof(T), alignof(T), alloc, CONTEXT);
	capacity *= 2;
}

template<typename T>
void heap<T>::push(T value) { 

	if(size == capacity) {
		grow();
	}

	memory[size] = value;
	size++;

	reheap_up(size - 1);
}

template<typename T>
T heap<T>::pop() { 

	LOG_DEBUG_ASSERT(size > 0);
	
	T ret = memory[0];
	size--;

	if(size > 0) {

		memory[0] = memory[size];
		reheap_down();
	}

	return ret;
}

template<typename T>
bool heap<T>::try_pop(T* out) { 

	if(!empty()) {

		*out = pop();
		return true;
	}

	return false;
}

template<typename T>
bool heap<T>::empty() { 

	return size == 0;
}

template<typename T>
void heap<T>::reheap_up(u32 node) { 

	if (!node) return;

	T val = memory[node];
	u32 p = (node - 1) / 2;
	T parent = memory[p];

	if(gt(val, parent)) {
		memory[node] = parent;
		memory[p] = val;
		reheap_up(p);
	}
}

template<typename T>
void heap<T>::reheap_down(u32 root) { 

	T val = memory[root];

	u32 l = root * 2 + 1;
	u32 r = root * 2 + 2;

	if(l < size && r < size) {
		T lv = memory[l];
		T rv = memory[r];
		if(gt(lv,val) && gt(lv,rv)) {
			memory[root] = lv;
			memory[l] = val;
			reheap_down(l);
		} else if(gt(rv,val) && gt(rv,lv)) {
			memory[root] = rv;
			memory[r] = val;
			reheap_down(r);
		}
	} else if(l < size) {
		T lv = memory[l];
		if(gt(lv,val)) {
			memory[root] = lv;
			memory[l] = val;
			reheap_down(l);
		}
	} else if(r < size) {
		T rv = memory[r];
		if(gt(rv,val)) {
			memory[root] = rv;
			memory[r] = val;
			reheap_down(r);
		}
	}
}

template<typename T>
locking_heap<T> locking_heap<T>::make(u32 capacity, allocator* alloc) { 

	locking_heap<T> ret;
	ret.capacity = capacity;
	
	if(!alloc) alloc = CURRENT_ALLOC();

	ret.alloc = alloc;
	PUSH_ALLOC(alloc) {
		ret.memory = NEWA(T, capacity);
	} POP_ALLOC();

	global_api->create_semaphore(&ret.sem, 0, INT_MAX);
	global_api->create_mutex(&ret.mut, false);

	return ret;
}

template<typename T>
void locking_heap<T>::destroy() { 

	heap<T>::destroy();
	global_api->destroy_mutex(&mut);
	global_api->destroy_semaphore(&sem);
}

template<typename T>
void locking_heap<T>::push(T value) { 

	global_api->aquire_mutex(&mut);
	heap<T>::push(value);
	global_api->release_mutex(&mut);
	global_api->signal_semaphore(&sem, 1);
}

template<typename T>
T locking_heap<T>::wait_pop() { 

	global_api->wait_semaphore(&sem, -1);
	T ret;
	try_pop(&ret);
	return ret;
}

template<typename T>
bool locking_heap<T>::try_pop(T* out) { 

	global_api->aquire_mutex(&mut);
	bool ret = heap<T>::try_pop(out);
	global_api->release_mutex(&mut);
	return ret;
}

template<typename T>
void locking_heap<T>::renew(float (*eval)(T,void*), void* param) { 

	global_api->aquire_mutex(&mut);
	heap<T>::renew(eval, param);
	global_api->release_mutex(&mut);
}
