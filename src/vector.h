
#pragma once

template<typename T>
struct vector {
	T* memory 	 = NULL;
	i32 size 	 = 0;
	i32 capacity = 0;
	allocator* alloc;
};

template<typename T>
vector<T> make_vector(i32 capacity, allocator* a) {

	vector<T> ret;

	ret.alloc = a;
	vector_resize(&ret, capacity);

	return ret;
}

template<typename T>
vector<T> make_vector(i32 capacity = 0) {

	vector<T> ret;

	ret.allocator = CURRENT_ALLOC();
	vector_resize(&ret, capacity);

	return ret;
}

template<typename T>
void vector_resize(vector<T>* v, i32 capacity) {

	T* new_memory = NULL;

	if(capacity > 0) {

		new_memory = (T*)v->alloc->allocate(capacity * sizeof(T), v->alloc);
	}

	if(v->memory && new_memory) {

		memcpy(v->memory, new_memory, v->capacity * sizeof(T));
	}

	if(v->memory) {

		v->alloc->free(v->memory, v->alloc);
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

		v->size++;
		v->memory[v->size] = value;

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
	T ret;
	return ret;
}

template<typename T>
T vector_back(vector<T>* v) { 

	if(v->size) {
		return v->memory[v->size];
	}

	// TODO(max): errors
	T ret;
	return ret;
}