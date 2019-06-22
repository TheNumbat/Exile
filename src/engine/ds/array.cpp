
#include "array.h"

template<typename T> 
u32 array<T>::len() { 

	return capacity;
}

template<typename T>
array<T> array<T>::make_copy(array<T>* src, allocator* a) { 

	array<T> ret = array<T>::make(src->capacity, a);
	_memcpy(src->memory, ret.memory, src->capacity * sizeof(T));

	return ret;
}

template<typename T>
void array<T>::destroy() { 

	if(alloc && memory) {

		alloc->free_(memory, capacity * sizeof(T), alloc, CONTEXT);
	}

	memory = null;
	capacity = 0;
}

template<typename T>
array<T> array<T>::make(u32 capacity, allocator* a) { 

	array<T> ret;

	PUSH_ALLOC(a) {

		ret.alloc = a;
		ret.capacity = capacity;

		if(capacity > 0) {

			ret.memory = NEWA(T, capacity);
		}

	} POP_ALLOC();
	
	return ret;
}

template<typename T>
array<T> array<T>::make(u32 capacity) { 

	array<T> ret;

	ret = array<T>::make(capacity, CURRENT_ALLOC());

	return ret;
}

template<typename T>
array<T> array<T>::make_memory(u32 capacity, void* memory) { 

	array<T> ret;

	ret.capacity = capacity;
	ret.memory = (T*)memory;
	
	return ret;
}

template<typename T>
inline T* array<T>::get(u32 idx) { 

#ifdef BOUNDS_CHECK
	if(memory && idx >= 0 && idx < capacity) {

		return memory + idx;
	} else {

		LOG_FATAL_F("array_get out of bounds, % < 0 || % > %"_, idx, idx, capacity);

		return null;
	}
#else
	return memory + idx;
#endif
}

