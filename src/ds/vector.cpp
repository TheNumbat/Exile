
template<typename T>
u32 vector_len(vector<T>* v) { PROF
	return v->size;
}

template<typename T>
u32 vector_capacity(vector<T>* v) { PROF
	return v->capacity;
}

template<typename T> 
void clear_vector(vector<T>* v) { PROF
	v->size = 0;
}

template<typename T>
T* vector_find(vector<T>* v, T val) { PROF

	T* ret = null;
	FORVEC(*v, 
		if(*it == val) {
			ret = it;
			break;
		}
	)

	return ret;
}

template<typename T>
void vector_erase(vector<T>* v, T val) { PROF

	FORVEC(*v, 
		if(*it == val) {
			vector_erase(v, __i);
			it--;
		}
	)
}

template<typename T>
u32 vector_partition(vector<T>* v, u32 low, u32 high) { PROF

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
void vector_qsort(vector<T>* v, u32 low, u32 high, bool first) { PROF
		
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
vector<T> make_vector_copy(vector<T> source, allocator* a) { PROF

	vector<T> ret = make_vector<T>(source.capacity, a);

	if(source.memory) {

		memcpy(source.memory, ret.memory, source.capacity * sizeof(T));
	}

	ret.size = source.size;

	return ret;	
}

template<typename T>
vector<T> make_vector_copy_trim(vector<T> source, allocator* a) { PROF

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

#ifdef MORE_PROF
	PROF
#endif

#ifdef BOUNDS_CHECK
	if(v->memory && idx >= 0 && idx < v->capacity) {
		return v->memory + idx;
	} else {
		
		LOG_FATAL_F("vector_get out of bounds, % < 0 || % >= %", idx, idx, v->capacity);
		return null;
	}
#else
	return v->memory + idx;
#endif
}

template<typename T>
void vector_grow(vector<T>* v, bool copy) { PROF
	
	vector_resize(v, v->capacity > 0 ? 2 * v->capacity : 4, copy);
}

template<typename T>
void destroy_vector(vector<T>* v) { PROF

	if(v->memory) {

		v->alloc->free_(v->memory, v->alloc, CONTEXT);
	}

	v->memory = null;
	v->size = 0;
	v->capacity = 0;
}

template<typename T>
vector<T> make_vector(u32 capacity, allocator* a) { PROF

	vector<T> ret;

	ret.alloc = a;
	vector_resize(&ret, capacity);

	return ret;
}

template<typename T>
vector<T> make_vector(u32 capacity) { PROF

	vector<T> ret;

	ret.alloc = CURRENT_ALLOC();
	vector_resize(&ret, capacity);

	return ret;
}

template<typename T>
void vector_resize(vector<T>* v, u32 capacity, bool copy) { PROF

	if(capacity == 0) {
		v->alloc->free_(v->memory, v->alloc, CONTEXT);
		v->memory = null;
		return;
	}

	if(v->memory)
		v->memory = (T*)v->alloc->reallocate_(v->memory, capacity * sizeof(T), v->alloc, CONTEXT);
	else
		v->memory = (T*)v->alloc->allocate_(capacity * sizeof(T), v->alloc, CONTEXT);
		
#ifdef CONSTRUCT_DS_ELEMENTS
	i64 added = capacity - v->capacity;
	if(added > 0) {
		T* new_place = v->memory + v->capacity;
		new (new_place) T[added]();
	}
#endif

	v->capacity = capacity;
}

template<typename T>
vector<T> make_vector_copy(vector<T> source) { PROF

	vector<T> ret = make_vector<T>(source.capacity, source.alloc);

	if(source.memory) {

		memcpy(source.memory, ret.memory, source.capacity * sizeof(T));
	}

	ret.size = source.size;

	return ret;
}

template<typename T>
T* vector_push(vector<T>* v, T value) {  

#ifdef MORE_PROF
	PROF
#endif

	if(v->size == v->capacity) {

		vector_grow(v);
	} 

	v->memory[v->size] = value;
	v->size++;

	return v->memory + v->size - 1;
}

template<typename T>
void vector_pop(vector<T>* v) { PROF 

	if(v->size > 0) {
		v->size--;
	}
}

template<typename T>
T* vector_front(vector<T>* v) { PROF 

	if(v->size > 0) {
		return v->memory;
	}

	LOG_FATAL("Trying to get empty vector front!");
	return null;
}

template<typename T>
T* vector_back(vector<T>* v) { PROF 

	if(v->size) {
		return v->memory + (v->size - 1);
	}

	LOG_FATAL("Trying to get empty vector back!");
	return null;
}

template<typename T>
void vector_erase(vector<T>* v, u32 index, u32 num) { PROF

#ifdef BOUNDS_CHECK
	if(v->size >= num) {
		if(index >= 0 && index < v->size) {
			for(u32 i = index + num; i < v->size; i++) {
				v->memory[i - num] = v->memory[i];
			}

			v->size -= num;
		} else {
			LOG_FATAL_F("vector_erase out of bounds % < 0 || % >= %", index, index, v->capacity);
		}
	} else {
		LOG_FATAL_F("vector_erase trying to erase % elements, % left", num, v->size);
	}
#else
	for(u32 i = index + num; i < v->size; i++) {
		v->memory[i - num] = v->memory[i];
	}
	v->size -= num;
#endif
}

template<typename T>
void vector_pop_front(vector<T>* v) { PROF

	vector_erase(v, 0);
}

template<typename T>
bool vector_empty(vector<T>* v) { PROF
	return v->size == 0;
}
