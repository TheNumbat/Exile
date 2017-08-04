
template<typename T>
void destroy_stack(stack<T>* s) { PROF

	destroy_vector(&s->contents);
}

template<typename T>
stack<T> make_stack_copy(stack<T> src, allocator* a) { PROF

	stack<T> ret;

	ret.contents = make_vector_copy(src.contents, a);

	return ret;
}

template<typename T>
stack<T> make_stack_copy_trim(stack<T> src, allocator* a) { PROF

	stack<T> ret;

	ret.contents = make_vector_copy_trim(src.contents, a);

	return ret;
}

template<typename T>
stack<T> make_stack(u32 capacity, allocator* a) { PROF

	stack<T> ret;

	ret.contents = make_vector<T>(capacity, a);
	
	return ret;
}

template<typename T> 
stack<T> make_stack_copy(stack<T> src) { PROF
	
	stack<T> ret;

	ret.contents = make_vector_copy(src.contents);

	return ret;
}

template<typename T>
stack<T> make_stack(u32 capacity) { PROF

	stack<T> ret;

	ret.contents = make_vector<T>(capacity);

	return ret;
}

template<typename T>
void clear_stack(stack<T>* s) { PROF

	vector_clear(&s->contents);
}

template<typename T>
void stack_push(stack<T>* s, T value) { PROF

	vector_push(&s->contents, value);
}

template<typename T>
T stack_pop(stack<T>* s) { PROF

	if(s->contents.size > 0) {
		
		T top = *stack_top(s);

		vector_pop(&s->contents);

		return top;	
	}

	LOG_FATAL("Trying to pop empty stack!");
	T ret = {};
	return ret;
}

template<typename T>
T* stack_top(stack<T>* s) { PROF

	if(s->contents.size > 0) {

		return vector_back(&s->contents);
	}

	LOG_FATAL("Trying to get top of empty stack!");
	return null;
}

template<typename T>
bool stack_empty(stack<T>* s) { PROF
	return vector_empty(&s->contents);
}

template<typename T>
bool stack_try_pop(stack<T>* s, T* out) {
	
	if(!stack_empty(s)) {
		
		*out = *stack_pop(s);
		return true;	
	}

	return false;
}

template<typename T>
con_stack<T> make_con_stack(u32 capacity, allocator* a) {

	con_stack<T> ret;
	ret.contents = make_vector(capacity, a);
	global_api->platform_create_mutex(&ret.mut, false);
	global_api->platform_create_semaphore(&ret.sem, 0, INT_MAX);

	return ret;
}

template<typename T>
con_stack<T> make_con_stack(u32 capacity) {

	return make_con_stack(capacity, CURRENT_ALLOC());
}

template<typename T>
void destroy_con_stack(con_stack<T>* s) {

	destroy_vector(&s->contents);
	global_api->platform_destroy_mutex(&s->mut);
	global_api->platform_destroy_semaphore(&s->sem);
}

template<typename T>
T* stack_push(con_stack<T>* s, T value) {

	global_api->platform_aquire_mutex(&s->mut, -1);
	T* ret = stack_push((stack<T>*)s, value);
	global_api->platform_release_mutex(&s->mut);
	global_api->platform_signal_semaphore(&s->sem, 1);
	return ret;
}

template<typename T>
T stack_wait_pop(con_stack<T>* s) {

	global_api->platform_wait_semaphore(&s->sem, -1);
	T ret;
	stack_try_pop(s, &ret);
	return ret;
}

template<typename T>
bool stack_try_pop(con_stack<T>* s, T* out) {

	global_api->platform_aquire_mutex(&s->mut, -1);
	bool ret = stack_try_pop((stack<T>*)s, out);
	global_api->platform_release_mutex(&s->mut);
	return ret;
}
