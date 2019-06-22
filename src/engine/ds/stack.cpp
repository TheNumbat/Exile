
#include "stack.h"

template<typename T>
void stack<T>::destroy() { 

	contents.destroy();
}

template<typename T>
stack<T> stack<T>::make_copy(stack<T> src, allocator* a) { 

	stack<T> ret;

	ret.contents = vector<T>::make_copy(src.contents, a);

	return ret;
}

template<typename T>
stack<T> stack<T>::make_copy_trim(stack<T> src, allocator* a) { 

	stack<T> ret;

	ret.contents = vector<T>::make_copy_trim(src.contents, a);

	return ret;
}

template<typename T>
stack<T> stack<T>::make(u32 capacity, allocator* a) { 

	stack<T> ret;

	ret.contents = vector<T>::make(capacity, a);
	
	return ret;
}

template<typename T> 
stack<T> stack<T>::make_copy(stack<T> src) { 
	
	stack<T> ret;

	ret.contents = vector<T>::make_copy(src.contents);

	return ret;
}

template<typename T>
stack<T> stack<T>::make(u32 capacity) { 

	stack<T> ret;

	ret.contents = vector<T>::make(capacity);

	return ret;
}

template<typename T>
void stack<T>::clear() { 

	contents.clear();
}

template<typename T>
void stack<T>::push(T value) { 

	contents.push(value);
}

template<typename T>
T stack<T>::pop() { 

	if(contents.size > 0) {
		
		T ret = *top();

		contents.pop();

		return ret;	
	}

	LOG_FATAL("Trying to pop empty stack!"_);
	T ret = {};
	return ret;
}

template<typename T>
T* stack<T>::top() { 

	if(contents.size > 0) {

		return contents.back();
	}

	LOG_FATAL("Trying to get top of empty stack!"_);
	return null;
}

template<typename T>
bool stack<T>::empty() { 
	return vector_empty(&contents);
}

template<typename T>
bool stack<T>::try_pop(T* out) { 
	
	if(!empty()) {
		
		*out = *pop();
		return true;	
	}

	return false;
}

template<typename T>
locking_stack<T> locking_stack<T>::make(u32 capacity, allocator* a) { 

	locking_stack<T> ret;
	ret.contents = vector<T>::make(capacity, a);
	global_api->create_mutex(&ret.mut, false);
	global_api->create_semaphore(&ret.sem, 0, INT_MAX);

	return ret;
}

template<typename T>
locking_stack<T> locking_stack<T>::make(u32 capacity) { 

	return locking_stack<T>::make(capacity, CURRENT_ALLOC());
}

template<typename T>
void locking_stack<T>::destroy() { 

	stack<T>::destroy();
	global_api->destroy_mutex(&mut);
	global_api->destroy_semaphore(&sem);
}

template<typename T>
T* locking_stack<T>::push(T value) { 

	global_api->aquire_mutex(&mut, -1);
	T* ret = stack<T>::push(value);
	global_api->release_mutex(&mut);
	global_api->signal_semaphore(&sem, 1);
	return ret;
}

template<typename T>
T locking_stack<T>::wait_pop() { 

	global_api->wait_semaphore(&sem, -1);
	T ret;
	try_pop(&ret);
	return ret;
}

template<typename T>
bool locking_stack<T>::try_pop(T* out) { 

	global_api->aquire_mutex(&mut, -1);
	bool ret = stack<T>::try_pop(out);
	global_api->release_mutex(&mut);
	return ret;
}
