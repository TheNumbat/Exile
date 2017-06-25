
template<typename T>
u32 vector_len(vector<T>* v) {
	return v->size;
}

template<typename T>
u32 vector_capacity(vector<T>* v) {
	return v->capacity;
}

template<typename T> 
void clear_vector(vector<T>* v) {
	v->size = 0;
}

template<typename T>
T* vector_find(vector<T>* v, T val) {

	T* ret = NULL;
	FORVEC(*v, 
		if(*it == val) {
			ret = it;
			break;
		}
	)

	return ret;
}

template<typename T>
void vector_erase(vector<T>* v, T val) {

	FORVEC(*v, 
		if(*it == val) {
			vector_erase(v, __i);
			it--;
		}
	)
}

template<typename T>
u32 vector_partition(vector<T>* v, u32 low, u32 high) {

	T pivot = v->memory[high];
	u32 i = low;

	for(u32 j = low; j < high; j++) {
		if(pivot < v->memory[j]) {

			i++;
			if(i != j) {
				T temp = v->memory[i];
				v->memory[i] = v->memory[j];
				v->memory[j] = temp;
			}
		}
	}
	return i;
}

template<typename T> 
void vector_qsort(vector<T>* v, u32 low, u32 high, bool first) {
		
	if (!v->size) return;

	if(first) {
		high = v->size - 1;
	}

	if(low < high) {

		u32 part = vector_partition(v, low, high);
		vector_qsort(v, low, part, false);
		vector_qsort(v, part + 1, high, false);
	}
}

template<typename T>
vector<T> make_vector_copy(vector<T> source, allocator* a) {

	vector<T> ret = make_vector<T>(source.capacity, a);

	if(source.memory) {

		memcpy(source.memory, ret.memory, source.capacity * sizeof(T));
	}

	ret.size = source.size;

	return ret;	
}

template<typename T>
vector<T> make_vector_copy_trim(vector<T> source, allocator* a) {

	vector<T> ret = make_vector<T>(source.size, a);

	if(source.memory) {

		memcpy(source.memory, ret.memory, source.size * sizeof(T));
	}

	ret.size = source.size;

	return ret;	
}

// operator[] but not a member
template<typename T>
inline T* vector_get(vector<T>* v, u32 idx) {

#ifdef BOUNDS_CHECK
	if(v->memory && idx >= 0 && idx < v->capacity) {
		return v->memory + idx;
	} else {
		
		LOG_FATAL_F("vector_get out of bounds, %u < 0 || %u >= %u", idx, idx, v->capacity);
		return NULL;
	}
#else
	return v->memory + idx;
#endif
}

template<typename T>
void vector_grow(vector<T>* v, bool copy) {
	
	vector_resize(v, v->capacity > 0 ? 2 * v->capacity : 4, copy);
}

template<typename T>
void destroy_vector(vector<T>* v) {

	if(v->memory) {

		v->alloc->free_(v->memory, v->alloc, CONTEXT);
	}

	v->memory = NULL;
	v->size = 0;
	v->capacity = 0;
}

template<typename T>
vector<T> make_vector(u32 capacity, allocator* a) {

	vector<T> ret;

	ret.alloc = a;
	vector_resize(&ret, capacity);

	return ret;
}

template<typename T>
vector<T> make_vector(u32 capacity) {

	vector<T> ret;

	ret.alloc = CURRENT_ALLOC();
	vector_resize(&ret, capacity);

	return ret;
}

template<typename T>
void vector_resize(vector<T>* v, u32 capacity, bool copy) {

	T* new_memory = NULL;

	if(capacity > 0) {

		new_memory = (T*)v->alloc->allocate_(capacity * sizeof(T), v->alloc, CONTEXT);
	}

	if(copy && v->memory && new_memory) {

		memcpy(v->memory, new_memory, v->capacity * sizeof(T));
	}

	if(v->memory) {

		v->alloc->free_(v->memory, v->alloc, CONTEXT);
	}
	
	v->memory = new_memory;
	v->capacity = capacity;
}

template<typename T>
vector<T> make_vector_copy(vector<T> source) {

	vector<T> ret = make_vector<T>(source.capacity, source.alloc);

	if(source.memory) {

		memcpy(source.memory, ret.memory, source.capacity * sizeof(T));
	}

	ret.size = source.size;

	return ret;
}

template<typename T>
void vector_push(vector<T>* v, T value) { 

	if(v->size == v->capacity) {

		vector_grow(v);
	} 

	v->memory[v->size] = value;
	v->size++;
}

template<typename T>
void vector_pop(vector<T>* v) { 

	if(v->size > 0) {
		v->size--;
	}
}

template<typename T>
T* vector_front(vector<T>* v) { 

	if(v->size > 0) {
		return v->memory;
	}

	LOG_FATAL("Trying to get empty vector front!");
	return NULL;
}

template<typename T>
T* vector_back(vector<T>* v) { 

	if(v->size) {
		return v->memory + (v->size - 1);
	}

	LOG_FATAL("Trying to get empty vector back!");
	return NULL;
}

template<typename T>
void vector_erase(vector<T>* v, u32 index, u32 num) {

#ifdef BOUNDS_CHECK
	if(v->size >= num) {
		if(index >= 0 && index < v->size) {
			for(u32 i = index + num; i < v->size; i++) {
				v->memory[i - num] = v->memory[i];
			}

			v->size -= num;
		} else {
			LOG_FATAL_F("vector_erase out of bounds %u < 0 || %u >= %u", index, index, v->capacity);
		}
	} else {
		LOG_FATAL_F("vector_erase trying to erase %u elements, %u left", num, v->size);
	}
#else
	for(u32 i = index + num; i < v->size; i++) {
		v->memory[i - num] = v->memory[i];
	}
	v->size -= num;
#endif
}

template<typename T>
void vector_pop_front(vector<T>* v) {

	vector_erase(v, 0);
}

template<typename T>
bool vector_empty(vector<T>* v) {
	return v->size == 0;
}
