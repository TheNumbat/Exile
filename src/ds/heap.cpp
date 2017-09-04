
template<typename T>
heap<T> heap<T>::make(u32 capacity, allocator* alloc) {

	heap<T> ret;
	ret.capacity = capacity;
	
	if(!alloc) alloc = CURRENT_ALLOC();

	ret.alloc = alloc;
	ret.memory = (T*)alloc->allocate_(capacity * sizeof(T), alloc, CONTEXT);

	return ret;
}	

template<typename T>
void heap<T>::destroy() {

	alloc->free_(memory, alloc, CONTEXT);

	memory = null;
	size = capacity = 0;
}

template<typename T>
void heap<T>::clear() {

	size = 0;
}

template<typename T>
void heap<T>::grow() {

	capacity *= 2;
	memory = (T*)alloc->reallocate_(memory, capacity * sizeof(T), alloc, CONTEXT);
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

	if(val > parent) {
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
		if(lv > val && lv > rv) {
			memory[root] = lv;
			memory[l] = val;
			reheap_down(l);
		} else if(rv > val && rv > lv) {
			memory[root] = rv;
			memory[r] = val;
			reheap_down(r);
		}
	} else if(l < size) {
		T lv = memory[l];
		if(lv > val) {
			memory[root] = lv;
			memory[l] = val;
			reheap_down(l);
		}
	} else if(r < size) {
		T rv = memory[r];
		if(rv > val) {
			memory[root] = rv;
			memory[r] = val;
			reheap_down(r);
		}
	}
}
