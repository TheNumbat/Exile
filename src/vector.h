
#pragma once

template<typename T>
struct vector {
	T* memory 	 	 = NULL;
	i32 size 	 	 = 0;
	i32 capacity 	 = 0;
	allocator* alloc = NULL;
};

template<typename T> vector<T> make_vector(i32 capacity, allocator* a);
template<typename T> vector<T> make_vector(i32 capacity = 4);
template<typename T> vector<T> make_vector_copy(vector<T> source);
template<typename T> void destroy_vector(vector<T>* v);

template<typename T> void vector_resize(vector<T>* v, i32 capacity);
template<typename T> void vector_push(vector<T>* v, T value);
template<typename T> void vector_pop(vector<T>* v);
template<typename T> void vector_pop_front(vector<T>* v);

template<typename T> T& get(vector<T>* v, i32 idx);
template<typename T> T vector_front(vector<T>* v);
template<typename T> T vector_back(vector<T>* v);

template<typename T> i32 vector_len(vector<T>* v);
template<typename T> i32 vector_capacity(vector<T>* v);
template<typename T> bool vector_empty(vector<T>* v);

template<typename T>
i32 vector_len(vector<T>* v) {
	return v->size;
}

template<typename T>
i32 vector_capacity(vector<T>* v) {
	return v->capacity;
}

// operator[] but not a member
template<typename T>
T& get(vector<T>* v, i32 idx) {

	if(v->memory && idx >= 0 && idx < v->size) {
		return v->memory[idx];
	}

	// TODO(max): error
	assert(false);
	T ret = {};
	return ret;
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
vector<T> make_vector(i32 capacity, allocator* a) {

	vector<T> ret;

	ret.alloc = a;
	vector_resize(&ret, capacity);

	return ret;
}

template<typename T>
vector<T> make_vector(i32 capacity) {

	vector<T> ret;

	ret.alloc = CURRENT_ALLOC();
	vector_resize(&ret, capacity);

	return ret;
}

template<typename T>
void vector_resize(vector<T>* v, i32 capacity) {

	T* new_memory = NULL;

	if(capacity > 0) {

		new_memory = (T*)v->alloc->allocate_(capacity * sizeof(T), v->alloc);
	}

	if(v->memory && new_memory) {

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

	vector ret = make_vector(source.capacity, source.alloc);

	if(source.memory) {

		memcpy(source.memroy, ret.memory, source.capacity * sizeof(T));
	}

	ret.size = source.size;

	return ret;
}

template<typename T>
void vector_push(vector<T>* v, T value) { 

	if(v->size < v->capacity) {

		v->memory[v->size] = value;
		v->size++;

	} else {

		vector_resize(v, v->capacity > 0 ? 2 * v->capacity : 4);
	
		v->size++;
		v->memory[v->size] = value;		
	}
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

	// TODO(max): errors
	assert(false);
	T ret;
	return ret;
}

template<typename T>
T vector_back(vector<T>* v) { 

	if(v->size) {
		return v->memory[v->size];
	}

	// TODO(max): errors
	assert(false);
	T ret = {};
	return ret;
}

template<typename T>
void vector_pop_front(vector<T>* v) {

	if(v->size > 0) {

		for(i32 i = 1; i < v->size; i++) {
			v->memory[i - 1] = v->memory[i];
		}

		v->size--;
	}
}

template<typename T>
bool vector_empty(vector<T>* v) {
	return v->size == 0;
}