
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
	FORVEC(it, *this) {
		if(*it == val) {
			ret = it;
			break;
		}
	}

	return ret;
}

template<typename T>
void vector<T>::erase(T val) { PROF

	FORVEC(it, *this) {
		if(*it == val) {
			erase(__it);
			it--;
		}
	}
}

template<typename T>
u32 vector<T>::partition(u32 low, u32 high) { PROF

	u32 pivot = low;
	for (u32 i = low + 1; i <= high; i++) {

		if (memory[i] < memory[pivot]) {
			T temp = memory[low];
			memory[low] = memory[i];
			memory[i] = temp;
			pivot++;
			low++;
		} else if(memory[pivot] < memory[i]) {
			T temp = memory[high];
			memory[high] = memory[i];
			memory[i] = temp;
			high--;
			i--;
		}
	}
	return pivot;
}

template<typename T>
u32 vector<T>::partition(bool (*comp)(T&,T&), u32 low, u32 high) { PROF

	u32 pivot = low;
	for (u32 i = low + 1; i <= high; i++) {

		if (comp(memory[i],memory[pivot])) {
			T temp = memory[low];
			memory[low] = memory[i];
			memory[i] = temp;
			pivot++;
			low++;
		} else if(comp(memory[pivot],memory[i])) {
			T temp = memory[high];
			memory[high] = memory[i];
			memory[i] = temp;
			high--;
			i--;
		}
	}
	return pivot;
}

template<typename T> 
void vector<T>::sort(u32 low, u32 high, bool first) { PROF
		
	if (!size) return;

	if(first) {
		high = size - 1;
	}

	if(low < high) {

		u32 part = partition(low, high);
		sort(low, part, false);
		sort(part + 1, high, false);
	}
}

template<typename T>
void vector<T>::sort(bool (*comp)(T&,T&), u32 low, u32 high, bool first) { PROF

	if (!size) return;

	if(first) {
		high = size - 1;
	}

	if(low < high) {

		u32 part = partition(comp, low, high);
		sort(comp, low, part, false);
		sort(comp, part + 1, high, false);
	}
}

template<typename T>
void vector<T>::merge(vector<T>& into, range l, range r) { PROF

	u32 i = l.l, j = r.l;
	u32 out = into.size;
	while(i < l.r && j < r.r) {
		if(memory[i] < memory[j]) {
			into[out++] = memory[j++];
		} else {
			into[out++] = memory[i++];
		}
	}
	while(i < l.r) {
		into[out++] = memory[i++];
	}
	while(j < l.r) {
		into[out++] = memory[j++];
	}
}

template<typename T> 
void vector<T>::mergesort(vector<T>& into, range r) { PROF

	if(r.l < r.r) {
		range one, two;
		one.l = r.l; 
		one.r = (r.l + r.r) / 2;
		two.l = one.r + 1;
		two.r = r.r;

		mergesort(into, one);
		mergesort(into, two);

		merge(into, one, two);
	}
}

template<typename T>
void vector<T>::stable_sort() { PROF

	vector<T> ret = make_copy(*this);
	
	range r;
	r.l = 0; r.r = size;
	// mergesort(ret, r);

	ret.size = size;
	destroy();
	*this = ret;
}

template<typename T>
void vector<T>::stable_sort(bool (*comp)(T&,T&)) { PROF

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

template<typename T>
T& vector<T>::operator[](u32 idx) {

	return *get(idx);
}

template<typename T>
inline T* vector<T>::get(u32 idx) { 

#ifdef MORE_PROF
	PROF
#endif

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
vector<T> vector<T>::make(u32 capacity) { PROF

	vector<T> ret;

	ret.alloc = CURRENT_ALLOC();
	ret.resize(capacity);

	return ret;
}

template<typename T>
void vector<T>::resize(u32 new_capacity) { PROF

	if(new_capacity == 0) {
		if(memory) {
			alloc->free_(memory, alloc, CONTEXT);
			memory = null;
		}
		return;
	}

	if(memory) {
		memory = (T*)alloc->reallocate_(memory, capacity * sizeof(T), new_capacity * sizeof(T), alloc, CONTEXT);

		i64 added = new_capacity - capacity;
		if(added > 0) {
			T* new_place = memory + capacity;
			new (new_place) T[added]();
		}
	} else {
		memory = (T*)alloc->allocate_(new_capacity * sizeof(T), alloc, CONTEXT);
		new (memory) T[new_capacity]();
	}

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

	if(size == capacity) {

		grow();
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
		
		return memory + size - 1;
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
