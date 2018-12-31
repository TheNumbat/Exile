
template<typename T>
void queue<T>::destroy() { 

	if(memory) {

		alloc->free_(memory, capacity * sizeof(T), alloc, CONTEXT);
	}

	memory = null;
	start = UINT32_MAX;
	end = 0;
	capacity = 0;
}

template<typename T>
void queue<T>::clear() { 

	start = UINT32_MAX;
	end = 0;
}

template<typename T>
queue<T> queue<T>::make(u32 capacity, allocator* a) { 

	queue<T> ret;

	ret.alloc = a;
	ret.capacity = capacity;
	if(capacity) {
		PUSH_ALLOC(a) {
			ret.memory = NEWA(T, capacity);
		} POP_ALLOC();
	}
	
	return ret;
}

template<typename T>
queue<T> queue<T>::make(u32 capacity) { 

	return queue<T>::make(capacity, CURRENT_ALLOC());
}

template<typename T>
u32 queue<T>::len() { 
	if(start == UINT32_MAX) return 0;
	i64 ret = (i64)end - (i64)start;
	if(ret <= 0) ret += capacity;
	return (u32)ret;
}

template<typename T>
void queue<T>::grow() { 
	
	u32 new_capacity = 2 * capacity;
	if(!new_capacity) new_capacity = 8;

	PUSH_ALLOC(alloc);

	if(memory) {

		u32 len = 0;
		T* new_mem = NEWA(T, new_capacity);
		u32 i = start;
		do {
			new_mem[len++] = memory[i];
			++i %= capacity;
		} while(i != end);
		start = 0;
		end = len;
		alloc->free_(memory, capacity * sizeof(T), alloc, CONTEXT);
		memory = new_mem;
	} else {
		memory = NEWA(T, new_capacity);
	}

	POP_ALLOC();

	capacity = new_capacity;
}

template<typename T>
T* queue<T>::push(T value) { 

	u32 size = len();
	if(size == 0) {
		start = end = 0;
	} else if(size == capacity) {
		grow();
	}

	T* ret = memory + end;
	*ret = value;

	++end %= capacity;
	return ret;
}

template<typename T>
bool queue<T>::full() {
	return len() == capacity;
}

template<typename T>
T queue<T>::push_overwrite(T value) { 

	T ret;
	if(len() == capacity) {
		ret = pop();
	}

	push(value);

	return ret;
}

template<typename T>
T queue<T>::pop() { 

	if(len() > 0) {
		
		T top = *front();
		++start %= capacity;

		if(start == end) start = UINT32_MAX;
		return top;	
	}

	LOG_FATAL("Trying to pop empty queue!"_);
	T ret = {};
	return ret;
}

template<typename T>
bool queue<T>::try_pop(T* out) { 

	if(len() > 0) {
	
		*out = pop();
		return true;	
	}

	return false;
}

template<typename T>
T* queue<T>::back() { 

	if(len() > 0) {

		i32 pos = end - 1;
		if(pos < 0) pos += capacity;
		return &memory[pos];
	}

	return null;
}

template<typename T>
T* queue<T>::front() { 

	if(len() > 0) {

		return &memory[start];
	}

	LOG_FATAL("Trying to get front of empty queue!"_);
	return null;
}

template<typename T>
bool queue<T>::empty() { 
	return len() == 0;
}

template<typename T>
locking_queue<T> locking_queue<T>::make(u32 capacity, allocator* a) { 

	locking_queue<T> ret;

	global_api->create_semaphore(&ret.sem, 0, INT_MAX);
	global_api->create_mutex(&ret.mut, false);

	ret.alloc = a;
	ret.capacity = capacity;
	if(capacity) {
		PUSH_ALLOC(a) {
			ret.memory = NEWA(T, capacity);
		} POP_ALLOC();
	}
	
	return ret;
}

template<typename T>
locking_queue<T> locking_queue<T>::make(u32 capacity) { 

	return locking_queue<T>::make(capacity, CURRENT_ALLOC());
}

template<typename T>
T* queue<T>::get(u32 idx) { 

#ifdef MORE_
	
#endif

#ifdef BOUNDS_CHECK
	LOG_ASSERT(start != UINT32_MAX);
	if(memory && idx >= 0 && idx < capacity) {
		return memory + (start + idx) % capacity;
	} else {
		
		LOG_FATAL_F("out of bounds, % < 0 || % >= %"_, idx, idx, capacity);
		return null;
	}
#else
	return memory + (start + idx) % capacity;
#endif
}

template<typename T>
void locking_queue<T>::destroy() { 

	queue<T>::destroy();
	global_api->destroy_mutex(&mut);
	global_api->destroy_semaphore(&sem);
}

template<typename T>
T* locking_queue<T>::push(T value) { 
	global_api->aquire_mutex(&mut);
	T* ret = queue<T>::push(value);
	global_api->release_mutex(&mut);
	global_api->signal_semaphore(&sem, 1);
	return ret;
}

template<typename T>
T locking_queue<T>::wait_pop() { 
	global_api->wait_semaphore(&sem, -1);
	T ret;
	try_pop(&ret);
	return ret;
}

template<typename T>
bool locking_queue<T>::try_pop(T* out) { 
	global_api->aquire_mutex(&mut);
	bool ret = queue<T>::try_pop(out);
	global_api->release_mutex(&mut);
	return ret;
}

template<typename T>
bool locking_queue<T>::empty() { 
	global_api->aquire_mutex(&mut);
	bool ret = queue<T>::empty();
	global_api->release_mutex(&mut);
	return ret;
}
