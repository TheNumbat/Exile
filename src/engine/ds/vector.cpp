
template<typename T> 
void vector<T>::clear() { 
	size = 0;
}

template<typename T>
void vector<T>::zero() { 
	_memset(memory, capacity * sizeof(T), 0);
}

template<typename T>
T* vector<T>::find(T val) { 

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
void vector<T>::erase(T val) { 

	FORVEC(it, *this) {
		if(*it == val) {
			erase(__it);
			it--;
		}
	}
}

template<typename T>
u32 vector<T>::partition(u32 low, u32 high) { 

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
u32 vector<T>::partition(bool (*less)(T,T), u32 low, u32 high) { 

	u32 pivot = low;
	for (u32 i = low + 1; i <= high; i++) {

		if (less(memory[i],memory[pivot])) {
			T temp = memory[low];
			memory[low] = memory[i];
			memory[i] = temp;
			pivot++;
			low++;
		} else if(less(memory[pivot],memory[i])) {
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
void vector<T>::sort(u32 low, u32 high, bool first) { 
		
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
void vector<T>::sort(bool (*less)(T,T), u32 low, u32 high, bool first) { 

	if (!size) return;

	if(first) {
		high = size - 1;
	}

	if(low < high) {

		u32 part = partition(less, low, high);
		sort(less, low, part, false);
		sort(less, part + 1, high, false);
	}
}

template<typename T>
void vector<T>::merge(u32 min, u32 mid, u32 max) { 

	u32 i = min, j = mid + 1, k = 0;
	vector<T> temp = make(max - min + 1);

	while(i <= mid && j <= max) {
		if(memory[i] <= memory[j]) {
			temp[k++] = memory[i++];
		} else {
			temp[k++] = memory[j++];
		}
	}
	while(i <= mid) {
		temp[k++] = memory[i++];
	}
	while(j <= max) {
		temp[k++] = memory[j++];
	}

	for (i = min; i <= max; i++) {
		memory[i] = temp[i - min];
	}	

	temp.destroy();
}

template<typename T> 
void vector<T>::mergesort(u32 min, u32 max) { 

	if(min < max) {
		u32 mid = (min + max) / 2;

		mergesort(min, mid);
		mergesort(mid + 1, max);

		merge(min, mid, max);
	}
}

template<typename T>
void vector<T>::stable_sort() { 

	if(size == 0) return;
	mergesort(0, size - 1);
}

template<typename T>
void vector<T>::merge(bool (*leq)(T,T), u32 min, u32 mid, u32 max) { 

	u32 i = min, j = mid + 1, k = 0;
	vector<T> temp = make(max - min + 1);

	while(i <= mid && j <= max) {
		if(leq(memory[i],memory[j])) {
			temp[k++] = memory[i++];
		} else {
			temp[k++] = memory[j++];
		}
	}
	while(i <= mid) {
		temp[k++] = memory[i++];
	}
	while(j <= max) {
		temp[k++] = memory[j++];
	}

	for (i = min; i <= max; i++) {
		memory[i] = temp[i - min];
	}	

	temp.destroy();
}

template<typename T> 
void vector<T>::mergesort(bool (*leq)(T,T), u32 min, u32 max) { 

	if(min < max) {
		u32 mid = (min + max) / 2;

		mergesort(leq, min, mid);
		mergesort(leq, mid + 1, max);

		merge(leq, min, mid, max);
	}
}

template<typename T>
void vector<T>::stable_sort(bool (*leq)(T,T)) { 

	if(size == 0) return;
	mergesort(leq, 0, size - 1);
}

template<typename T>
vector<T> vector<T>::make_copy(vector<T> source, allocator* a) { 

	vector<T> ret = vector<T>::make(source.capacity, a);

	if(source.memory) {

		memcpy(source.memory, ret.memory, source.capacity * sizeof(T));
	}

	ret.size = source.size;

	return ret;	
}

template<typename T>
vector<T> vector<T>::make_copy_trim(vector<T> source, allocator* a) { 

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

#ifdef BOUNDS_CHECK
	if(memory && idx >= 0 && idx < capacity) {
		return memory + idx;
	} else {
		
		LOG_FATAL_F("out of bounds, % < 0 || % >= %"_, idx, idx, capacity);
		return null;
	}
#else
	return memory + idx;
#endif
}

template<typename T>
void vector<T>::grow() { 
	
	resize(capacity > 0 ? 2 * capacity : 8);
}

template<typename T>
void vector<T>::destroy() { 

	if(memory) {

		alloc->free_(memory, capacity * sizeof(T), alloc, CONTEXT);
	}

	memory = null;
	size = 0;
	capacity = 0;
}

template<typename T>
vector<T> vector<T>::make(u32 capacity, allocator* a) { 

	vector<T> ret;

	ret.alloc = a;
	ret.resize(capacity);

	return ret;
}

template<typename T>
vector<T> vector<T>::make(u32 capacity) { 

	vector<T> ret;

	ret.alloc = CURRENT_ALLOC();
	ret.resize(capacity);

	return ret;
}

template<typename T>
void vector<T>::resize(u32 new_capacity) { 

	if(new_capacity == 0) {
		if(memory) {
			alloc->free_(memory, capacity * sizeof(T), alloc, CONTEXT);
			memory = null;
		}
		capacity = size = 0;
		return;
	}

	if(memory) {
		memory = (T*)alloc->reallocate_(memory, capacity * sizeof(T), new_capacity * sizeof(T), alignof(T), alloc, CONTEXT);

		i64 added = new_capacity - capacity;
		if(added > 0) {
			T* new_place = memory + capacity;
			new (new_place) T[added]();
		}
	} else {
		memory = (T*)alloc->allocate_(new_capacity * sizeof(T), alignof(T), alloc, CONTEXT);
		new (memory) T[new_capacity]();
	}

	capacity = new_capacity;
}

template<typename T>
vector<T> vector<T>::make_copy(vector<T> source) { 

	vector<T> ret = vector<T>::make(source.capacity, source.alloc);

	if(source.memory) {

		_memcpy(source.memory, ret.memory, source.capacity * sizeof(T));
	}

	ret.size = source.size;

	return ret;
}

template<typename T>
T* vector<T>::push(T value) { 

	if(size == capacity) {

		grow();
	} 

	memory[size] = value;
	size++;

	return memory + size - 1;
}


template<typename T>
void vector<T>::pop() {  

	if(size > 0) {
		size--;
	}
}

template<typename T>
T* vector<T>::front() {  

	if(size > 0) {
		return memory;
	}

	LOG_FATAL("Trying to get empty vector front!"_);
	return null;
}

template<typename T>
T* vector<T>::back() {  

	if(size) {
		
		return memory + size - 1;
	}

	LOG_FATAL("Trying to get empty vector back!"_);
	return null;
}

template<typename T>
void vector<T>::erase(u32 index, u32 num) { 

#ifdef BOUNDS_CHECK
	if(size >= num) {
		if(index >= 0 && index < size) {
			for(u32 i = index + num; i < size; i++) {
				memory[i - num] = memory[i];
			}

			size -= num;
		} else {
			LOG_FATAL_F("vector_erase out of bounds % < 0 || % >= %"_, index, index, capacity);
		}
	} else {
		LOG_FATAL_F("vector_erase trying to erase % elements, % left"_, num, size);
	}
#else
	for(u32 i = index + num; i < size; i++) {
		memory[i - num] = memory[i];
	}
	size -= num;
#endif
}

template<typename T>
void vector<T>::pop_front() { 

	erase(0);
}

template<typename T>
bool vector<T>::empty() { 
	return size == 0;
}
