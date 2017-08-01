
template<typename T>
void destroy_queue(queue<T>* q) { PROF

	if(q->memory) {

		q->alloc->free_(q->memory, q->alloc, CONTEXT);
	}

	q->memory = null;
	q->start = 0;
	q->end = 0;
	q->capacity = 0;
}

template<typename T>
queue<T> make_queue(u32 capacity, allocator* a) { PROF

	queue<T> ret;
	capacity = 0;

	ret.alloc = a;
	ret.capacity = capacity;
	if(capacity)
		ret.memory = (T*)a->allocate_(capacity * sizeof(T), a, CONTEXT);

#ifdef CONSTRUCT_DS_ELEMENTS
	new (ret.memory) T[capacity]();
#endif
	
	return ret;
}

template<typename T>
queue<T> make_queue(u32 capacity) { PROF

	return make_queue(capacity, CURRENT_ALLOC());
}

template<typename T>
u32 queue_len(queue<T>* q) { PROF
	i64 ret = (i64)q->end - (i64)q->start;
	if(ret < 0) ret += q->capacity;
	return (u32)ret;
}

template<typename T>
void queue_grow(queue<T>* q) { PROF
	
	u32 capacity = 2 * q->capacity;
	if(!capacity) capacity = 8;

	if(q->memory) {

		u32 len = 0;
		T* new_mem = (T*)q->alloc->allocate_(capacity * sizeof(T), q->alloc, CONTEXT);
		for(u32 i = q->start; i < q->end; ++i %= q->capacity) {
			new_mem[len++] = q->memory[i];
		}
		q->start = 0;
		q->end = len;
		q->alloc->free_(q->memory, q->alloc, CONTEXT);
		q->memory = new_mem;

#ifdef CONSTRUCT_DS_ELEMENTS
		new (q->memory + len) T[capacity - q->capacity]();
#endif		

	} else {
		q->memory = (T*)q->alloc->allocate_(capacity * sizeof(T), q->alloc, CONTEXT);

#ifdef CONSTRUCT_DS_ELEMENTS
		new (q->memory) T[capacity]();
#endif
	}

	q->capacity = capacity;
}

template<typename T>
T* queue_push(queue<T>* q, T value) { PROF

	if(queue_len(q) >= q->capacity) {
		queue_grow(q);
	}

	T* ret = q->memory + q->end;
	*ret = value;

	++q->end %= q->capacity;
	return ret;
}

template<typename T>
T queue_pop(queue<T>* q) { PROF

	if(queue_len(q) > 0) {
		
		T top = *queue_front(q);
		++q->start %= q->capacity;
		return top;	
	}

	LOG_FATAL("Trying to pop empty queue!");
	T ret = {};
	return ret;
}

template<typename T>
T* queue_back(queue<T>* q) { PROF

	if(queue_len(q) > 0) {

		return &q->memory[q->end - 1];
	}

	LOG_FATAL("Trying to get back of empty queue!");
	return null;
}

template<typename T>
T* queue_front(queue<T>* q) { PROF

	if(queue_len(q) > 0) {

		return &q->memory[q->start];
	}

	LOG_FATAL("Trying to get front of empty queue!");
	return null;
}

template<typename T>
bool queue_empty(queue<T>* q) { PROF
	return queue_len(q) == 0;
}
