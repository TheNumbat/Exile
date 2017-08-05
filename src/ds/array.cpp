
template<typename T> 
u32 array<T>::len() { PROF

	return capacity;
}

template<typename T>
array<T> array<T>::make_copy(array<T>* src, allocator* a) { PROF

	array<T> ret = array<T>::make(src->capacity, a);
	memcpy(src->memory, ret.memory, src->capacity * sizeof(T));

	return ret;
}

template<typename T>
void array<T>::destroy() { PROF

	if(alloc && memory) {

		alloc->free_(memory, alloc, CONTEXT);
	}

	memory = null;
	capacity = 0;
}

template<typename T>
array<T> array<T>::make(u32 capacity, allocator* a) { PROF

	array<T> ret;

	ret.alloc = a;
	ret.capacity = capacity;

	if(capacity > 0) {

		ret.memory = (T*)ret.alloc->allocate_(capacity * sizeof(T), ret.alloc, CONTEXT);

#ifdef CONSTRUCT_DS_ELEMENTS
		new (ret.memory) T[capacity]();
#endif
	}
	
	return ret;
}

template<typename T>
array<T> array<T>::make(u32 capacity) { PROF

	array<T> ret;

	ret = array<T>::make(capacity, CURRENT_ALLOC());

	return ret;
}

template<typename T>
array<T> array<T>::make_memory(u32 capacity, void* memory) { PROF

	array<T> ret;

	ret.capacity = capacity;
	ret.memory = (T*)memory;
	
	return ret;
}

// operator[] but not a member
template<typename T>
inline T* array<T>::get(u32 idx) {

#ifdef MORE_PROF
	PROF
#endif

#ifdef BOUNDS_CHECK
	if(memory && idx >= 0 && idx < capacity) {

		return memory + idx;
	} else {

		LOG_FATAL_F("array_get out of bounds, % < 0 || % > %", idx, idx, capacity);

		return null;
	}
#else
	return memory + idx;
#endif
}

