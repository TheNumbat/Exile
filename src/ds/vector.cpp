
template<typename T> 
void vector<T>::clear() { PROF
	size = 0;
}

template<typename T>
void vector<T>::zero() { PROF
	memset(memory, capacity * sizeof(T), 0);
}

template<typename T>
T* vector<T>::find(T val) { PROF

	T* ret = null;
	FORVEC(*this, 
		if(*it == val) {
			ret = it;
			break;
		}
	)

	return ret;
}

template<typename T>
void vector<T>::erase(T val) { PROF

	FORVEC(*this, 
		if(*it == val) {
			erase(__i);
			it--;
		}
	)
}

template<typename T>
u32 vector<T>::partition(u32 low, u32 high) { PROF

	T pivot = memory[high];
	u32 i = low;

	for(u32 j = low; j < high; j++) {
		if(pivot < memory[j]) {

			i++;
			if(i != j) {
				T temp = memory[i];
				memory[i] = memory[j];
				memory[j] = temp;
			}
		}
	}
	return i;
}

template<typename T> 
void vector<T>::qsort(u32 low, u32 high, bool first) { PROF
		
	if (!size) return;

	if(first) {
		high = size - 1;
	}

	if(low < high) {

		u32 part = partition(low, high);
		qsort(low, part, false);
		qsort(part + 1, high, false);
	}
}

template<typename T>
vector<T> vector<T>::make_copy(vector<T> source, allocator* a) { PROF

	vector<T> ret = vector<T>::make(source.capacity, a);

	if(source.memory) {

		memcpy(source.memory, ret.memory, source.capacity * sizeof(T));
	}

	ret.size = source.size;

	return ret;	
}

template<typename T>
vector<T> vector<T>::make_copy_trim(vector<T> source, allocator* a) { PROF

	vector<T> ret = vector<T>::make(source.size, a);

	if(source.memory) {

		memcpy(source.memory, ret.memory, source.size * sizeof(T));
	}

	ret.size = source.size;

	return ret;	
}

// operator[] but not a member
template<typename T>
inline T* vector<T>::get(u32 idx) { 

#ifdef MORE_PROF
	PROF
#endif

	return get_noprof(idx);
}

template<typename T>
inline T* vector<T>::get_noprof(u32 idx) { 

#ifdef BOUNDS_CHECK
	if(memory && idx >= 0 && idx < capacity) {
		return memory + idx;
	} else {
		
		LOG_FATAL_F("out of bounds, % < 0 || % >= %", idx, idx, capacity);
		return null;
	}
#else
	return memory + idx;
#endif
}

template<typename T>
void vector<T>::grow() { PROF
	
	resize(capacity > 0 ? 2 * capacity : 8);
}

template<typename T>
void vector<T>::grow_noprof() {
	
	resize_noprof(capacity > 0 ? 2 * capacity : 8);
}

template<typename T>
void vector<T>::destroy() { PROF

	if(memory) {

		alloc->free_(memory, alloc, CONTEXT);
	}

	memory = null;
	size = 0;
	capacity = 0;
}

template<typename T>
vector<T> vector<T>::make(u32 capacity, allocator* a) { PROF

	vector<T> ret;

	ret.alloc = a;
	ret.resize(capacity);

	return ret;
}

template<typename T>
vector<T> vector<T>::make_noprof(u32 capacity, allocator* a) {

	vector<T> ret;

	ret.alloc = a;
	ret.resize_noprof(capacity);

	return ret;
}

template<typename T>
vector<T> vector<T>::make(u32 capacity) { PROF

	vector<T> ret;

	ret.alloc = CURRENT_ALLOC();
	ret.resize(capacity);

	return ret;
}

template<typename T>
void vector<T>::resize(u32 new_capacity) { PROF

	resize_noprof(new_capacity);
}

template<typename T>
void vector<T>::resize_noprof(u32 new_capacity) {

	if(new_capacity == 0) {
		if(memory) {
			alloc->free_(memory, alloc, CONTEXT);
			memory = null;
		}
		return;
	}

	if(memory)
		memory = (T*)alloc->reallocate_(memory, new_capacity * sizeof(T), alloc, CONTEXT);
	else
		memory = (T*)alloc->allocate_(new_capacity * sizeof(T), alloc, CONTEXT);
		
#ifdef CONSTRUCT_DS_ELEMENTS
	i64 added = new_capacity - capacity;
	if(added > 0) {
		T* new_place = memory + capacity;
		new (new_place) T[added]();
	}
#endif

	capacity = new_capacity;
}

template<typename T>
vector<T> vector<T>::make_copy(vector<T> source) { PROF

	vector<T> ret = vector<T>::make(source.capacity, source.alloc);

	if(source.memory) {

		memcpy(source.memory, ret.memory, source.capacity * sizeof(T));
	}

	ret.size = source.size;

	return ret;
}

template<typename T>
T* vector<T>::push(T value) { 

#ifdef MORE_PROF
	PROF
#endif

	return push_noprof(value);
}

template<typename T>
T* vector<T>::push_noprof(T value) {  

	if(size == capacity) {

		grow_noprof();
	} 

	memory[size] = value;
	size++;

	return memory + size - 1;
}


template<typename T>
void vector<T>::pop() { PROF 

	if(size > 0) {
		size--;
	}
}

template<typename T>
T* vector<T>::front() { PROF 

	if(size > 0) {
		return memory;
	}

	LOG_FATAL("Trying to get empty vector front!");
	return null;
}

template<typename T>
T* vector<T>::back() { PROF 

	if(size) {
		return memory + (size - 1);
	}

	LOG_FATAL("Trying to get empty vector back!");
	return null;
}

template<typename T>
void vector<T>::erase(u32 index, u32 num) { PROF

#ifdef BOUNDS_CHECK
	if(size >= num) {
		if(index >= 0 && index < size) {
			for(u32 i = index + num; i < size; i++) {
				memory[i - num] = memory[i];
			}

			size -= num;
		} else {
			LOG_FATAL_F("vector_erase out of bounds % < 0 || % >= %", index, index, capacity);
		}
	} else {
		LOG_FATAL_F("vector_erase trying to erase % elements, % left", num, size);
	}
#else
	for(u32 i = index + num; i < size; i++) {
		memory[i - num] = memory[i];
	}
	size -= num;
#endif
}

template<typename T>
void vector<T>::pop_front() { PROF

	erase(0);
}

template<typename T>
bool vector<T>::empty() { PROF
	return size == 0;
}
