
template<typename T>
void queue<T>::destroy() { PROF

	if(memory) {

		alloc->free_(memory, alloc, CONTEXT);
	}

	memory = null;
	start = 0;
	end = 0;
	capacity = 0;
}

template<typename T>
queue<T> queue<T>::make(u32 capacity, allocator* a) { PROF

	queue<T> ret;

	ret.alloc = a;
	ret.capacity = capacity;
	if(capacity) {
		ret.memory = (T*)ret.alloc->allocate_(capacity * sizeof(T), ret.alloc, CONTEXT);

#ifdef CONSTRUCT_DS_ELEMENTS
		new (ret.memory) T[capacity]();
#endif
	}
	
	return ret;
}

template<typename T>
queue<T> queue<T>::make(u32 capacity) { PROF

	return queue<T>::make(capacity, CURRENT_ALLOC());
}


template<typename T>
u32 queue<T>::len() { PROF
	i64 ret = (i64)end - (i64)start;
	if(ret < 0) ret += capacity;
	return (u32)ret;
}

template<typename T>
void queue<T>::grow() { PROF
	
	u32 new_capacity = 2 * capacity;
	if(!new_capacity) new_capacity = 8;

	if(memory) {

		u32 len = 0;
		T* new_mem = (T*)alloc->allocate_(new_capacity * sizeof(T), alloc, CONTEXT);
		for(u32 i = start; i != end; ++i %= new_capacity) {
			new_mem[len++] = memory[i];
		}
		start = 0;
		end = len;
		alloc->free_(memory, alloc, CONTEXT);
		memory = new_mem;

#ifdef CONSTRUCT_DS_ELEMENTS
		new (memory + len) T[new_capacity - new_capacity]();
#endif		

	} else {
		memory = (T*)alloc->allocate_(new_capacity * sizeof(T), alloc, CONTEXT);

#ifdef CONSTRUCT_DS_ELEMENTS
		new (memory) T[new_capacity]();
#endif
	}

	capacity = new_capacity;
}

template<typename T>
T* queue<T>::push(T value) { PROF

	if(len() + 1 >= capacity) {
		grow();
	}

	T* ret = memory + end;
	*ret = value;

	++end %= capacity;
	return ret;
}

template<typename T>
T queue<T>::pop() { PROF

	if(len() > 0) {
		
		T top = *front();
		++start %= capacity;
		return top;	
	}

	LOG_FATAL("Trying to pop empty queue!");
	T ret = {};
	return ret;
}

template<typename T>
bool queue<T>::try_pop(T* out) { PROF

	if(len() > 0) {
	
		*out = *front();
		++start %= capacity;
		return true;	
	}

	return false;
}

template<typename T>
T* queue<T>::back() { PROF

	if(len() > 0) {

		return &memory[end - 1];
	}

	LOG_FATAL("Trying to get back of empty queue!");
	return null;
}

template<typename T>
T* queue<T>::front() { PROF

	if(len() > 0) {

		return &memory[start];
	}

	LOG_FATAL("Trying to get front of empty queue!");
	return null;
}

template<typename T>
bool queue<T>::empty() { PROF
	return start == end;
}



template<typename T>
con_queue<T> con_queue<T>::make(u32 capacity, allocator* a) { PROF

	con_queue<T> ret;

	global_api->platform_create_semaphore(&ret.sem, 0, INT_MAX);
	global_api->platform_create_mutex(&ret.mut, false);

	ret.alloc = a;
	ret.capacity = capacity;
	if(capacity) {
		ret.memory = (T*)ret.alloc->allocate_(capacity * sizeof(T), ret.alloc, CONTEXT);

#ifdef CONSTRUCT_DS_ELEMENTS
		new (ret.memory) T[capacity]();
#endif
	}
	
	return ret;
}

template<typename T>
con_queue<T> con_queue<T>::make(u32 capacity) { PROF

	return make_con_queue<T>(capacity, CURRENT_ALLOC());
}

template<typename T>
void con_queue<T>::destroy() { PROF

	((queue<T>*)this)->destroy(); // TODO(max): this is super kludgy
	global_api->platform_destroy_mutex(&mut);
	global_api->platform_destroy_semaphore(&sem);
}

template<typename T>
T* con_queue<T>::push(T value) { PROF
	global_api->platform_aquire_mutex(&mut, -1);
	T* ret = ((queue<T>*)this)->push(value);
	global_api->platform_release_mutex(&mut);
	global_api->platform_signal_semaphore(&sem, 1);
	return ret;
}

template<typename T>
T con_queue<T>::wait_pop() { PROF
	global_api->platform_wait_semaphore(&sem, -1);
	T ret;
	try_pop(&ret);
	return ret;
}

template<typename T>
bool con_queue<T>::try_pop(T* out) { PROF
	global_api->platform_aquire_mutex(&mut, -1);
	bool ret = ((queue<T>*)this)->try_pop(out);
	global_api->platform_release_mutex(&mut);
	return ret;
}
