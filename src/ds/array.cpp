
template<typename T> 
u32 array_len(array<T>* a) {
	return a->capacity;
}

template<typename T>
void destroy_array(array<T>* a) {

	if(a->alloc && a->memory) {

		a->alloc->free_(a->memory, a->alloc, CONTEXT);
	}

	a->memory = NULL;
	a->capacity = 0;
}

template<typename T>
array<T> make_array(u32 capacity, allocator* a) {

	array<T> ret;

	ret.alloc = a;
	ret.capacity = capacity;

	if(capacity > 0) {

		ret.memory = (T*)ret.alloc->allocate_(capacity * sizeof(T), ret.alloc, CONTEXT);
	}
	
	return ret;
}

template<typename T>
array<T> make_array(u32 capacity) {

	array<T> ret;

	ret = make_array<T>(capacity, CURRENT_ALLOC());

	return ret;
}

template<typename T>
array<T> make_array_memory(u32 capacity, void* memory) {

	array<T> ret;

	ret.capacity = capacity;
	ret.memory = (T*)memory;

	return ret;
}

// operator[] but not a member
template<typename T>
inline T* array_get(array<T>* a, u32 idx) {

#ifdef BOUNDS_CHECK
	if(a->memory && idx >= 0 && idx < a->capacity) {

		return a->memory + idx;
	} else {

		LOG_FATAL_F("array_get out of bounds, %u < 0 || %u > %u", idx, idx, a->capacity);
		return NULL;
	}
#else
	return a->memory + idx;
#endif
}

