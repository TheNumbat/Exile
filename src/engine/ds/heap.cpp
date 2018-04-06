
template<typename T, bool(comp)(T,T)>
heap<T,comp> heap<T,comp>::make(u32 capacity, allocator* alloc) { PROF

	heap<T,comp> ret;
	ret.capacity = capacity;
	
	if(!alloc) alloc = CURRENT_ALLOC();

	ret.alloc = alloc;

	PUSH_ALLOC(alloc) {
		ret.memory = NEWA(T, capacity);
	} POP_ALLOC();

	return ret;
}	

template<typename T, bool(comp)(T,T)>
void heap<T,comp>::destroy() { PROF

	if(memory) {
		alloc->free_(memory, capacity * sizeof(T), alloc, CONTEXT); 
		memory = null;
	}

	size = capacity = 0;
}

template<typename T, bool(comp)(T,T)>
void heap<T,comp>::clear() { PROF

	size = 0;
}

template<typename T, bool(comp)(T,T)>
void heap<T,comp>::grow() { PROF

	memory = (T*)alloc->reallocate_(memory, capacity * sizeof(T), 2 * capacity * sizeof(T), alignof(T), alloc, CONTEXT);
	capacity *= 2;
}

template<typename T, bool(comp)(T,T)>
void heap<T,comp>::push(T value) { PROF

	if(size == capacity) {
		grow();
	}

	memory[size] = value;
	size++;

	reheap_up(size - 1);
}

template<typename T, bool(comp)(T,T)>
T heap<T,comp>::pop() { PROF

	LOG_DEBUG_ASSERT(size > 0);
	
	T ret = memory[0];
	size--;

	if(size > 0) {

		memory[0] = memory[size];
		reheap_down();
	}

	return ret;
}

template<typename T, bool(comp)(T,T)>
bool heap<T,comp>::try_pop(T* out) { PROF

	if(!empty()) {

		*out = pop();
		return true;
	}

	return false;
}

template<typename T, bool(comp)(T,T)>
bool heap<T,comp>::empty() { PROF

	return size == 0;
}

template<typename T, bool(comp)(T,T)>
void heap<T,comp>::reheap_up(u32 node) { PROF

	if (!node) return;

	T val = memory[node];
	u32 p = (node - 1) / 2;
	T parent = memory[p];

	if(comp(val, parent)) {
		memory[node] = parent;
		memory[p] = val;
		reheap_up(p);
	}
}

template<typename T, bool(comp)(T,T)>
void heap<T,comp>::reheap_down(u32 root) { PROF

	T val = memory[root];

	u32 l = root * 2 + 1;
	u32 r = root * 2 + 2;

	if(l < size && r < size) {
		T lv = memory[l];
		T rv = memory[r];
		if(comp(lv,val) && comp(lv,rv)) {
			memory[root] = lv;
			memory[l] = val;
			reheap_down(l);
		} else if(comp(rv,val) && comp(rv,lv)) {
			memory[root] = rv;
			memory[r] = val;
			reheap_down(r);
		}
	} else if(l < size) {
		T lv = memory[l];
		if(comp(lv,val)) {
			memory[root] = lv;
			memory[l] = val;
			reheap_down(l);
		}
	} else if(r < size) {
		T rv = memory[r];
		if(comp(rv,val)) {
			memory[root] = rv;
			memory[r] = val;
			reheap_down(r);
		}
	}
}

template<typename T, bool(comp)(T,T)>
locking_heap<T,comp> locking_heap<T,comp>::make(u32 capacity, allocator* alloc) { PROF

	locking_heap<T,comp> ret;
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

template<typename T, bool(comp)(T,T)>
void locking_heap<T,comp>::destroy() { PROF

	heap<T,comp>::destroy();
	global_api->destroy_mutex(&mut);
	global_api->destroy_semaphore(&sem);
}

template<typename T, bool(comp)(T,T)>
void locking_heap<T,comp>::push(T value) { PROF

	global_api->aquire_mutex(&mut);
	heap<T,comp>::push(value);
	global_api->release_mutex(&mut);
	global_api->signal_semaphore(&sem, 1);
}

template<typename T, bool(comp)(T,T)>
T locking_heap<T,comp>::wait_pop() { PROF

	global_api->wait_semaphore(&sem, -1);
	T ret;
	try_pop(&ret);
	return ret;
}

template<typename T, bool(comp)(T,T)>
bool locking_heap<T,comp>::try_pop(T* out) { PROF

	global_api->aquire_mutex(&mut);
	bool ret = heap<T,comp>::try_pop(out);
	global_api->release_mutex(&mut);
	return ret;
}

template<typename T, bool(comp)(T,T)>
void locking_heap<T,comp>::renew(float (*eval)(T,void*), void* param) { PROF

	global_api->aquire_mutex(&mut);
	heap<T,comp>::renew(eval, param);
	global_api->release_mutex(&mut);
}
