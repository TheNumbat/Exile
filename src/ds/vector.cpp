
template<typename T>
u32 vector_len(vector<T>* v) {
	return v->size;
}

template<typename T>
u32 vector_capacity(vector<T>* v) {
	return v->capacity;
}

// operator[] but not a member
template<typename T>
inline T* vector_get(vector<T>* v, u32 idx) {

#ifdef BOUNDS_CHECK
	if(v->memory && idx >= 0 && idx < v->capacity) {
		return v->memory + idx;
	} else {
		
		LOG_FATAL_F("vector_get out of bounds, %u < 0 || %u > %u", idx, idx, v->capacity);
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

		v->alloc->free_(v->memory, v->alloc);
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

		new_memory = (T*)v->alloc->allocate_(capacity * sizeof(T), v->alloc);
	}

	if(copy && v->memory && new_memory) {

		memcpy(v->memory, new_memory, v->capacity * sizeof(T));
	}

	if(v->memory) {

		v->alloc->free_(v->memory, v->alloc);
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
T vector_front(vector<T>* v) { 

	if(v->size) {
		return v->memory[0];
	}

	LOG_FATAL("Trying to get empty vector back!");
	T ret;
	return ret;
}

template<typename T>
T vector_back(vector<T>* v) { 

	if(v->size) {
		return v->memory[v->size];
	}

	LOG_FATAL("Trying to get empty vector back!");
	T ret = {};
	return ret;
}

template<typename T>
void vector_pop_front(vector<T>* v) {

	if(v->size > 0) {

		for(u32 i = 1; i < v->size; i++) {
			v->memory[i - 1] = v->memory[i];
		}

		v->size--;
	}
}

template<typename T>
bool vector_empty(vector<T>* v) {
	return v->size == 0;
}
