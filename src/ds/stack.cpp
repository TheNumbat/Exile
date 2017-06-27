
template<typename T>
void destroy_stack(stack<T>* s) { FUNC

	destroy_vector(&s->contents);
}

template<typename T>
stack<T> make_stack_copy(stack<T> src, allocator* a) { FUNC

	stack<T> ret;

	ret.contents = make_vector_copy(src.contents, a);

	return ret;
}

template<typename T>
stack<T> make_stack_copy_trim(stack<T> src, allocator* a) { FUNC

	stack<T> ret;

	ret.contents = make_vector_copy_trim(src.contents, a);

	return ret;
}

template<typename T>
stack<T> make_stack(u32 capacity, allocator* a) { FUNC

	stack<T> ret;

	ret.contents = make_vector<T>(capacity, a);
	
	return ret;
}

template<typename T> 
stack<T> make_stack_copy(stack<T> src) { FUNC
	
	stack<T> ret;

	ret.contents = make_vector_copy(src.contents);

	return ret;
}

template<typename T>
stack<T> make_stack(u32 capacity) { FUNC

	stack<T> ret;

	ret.contents = make_vector<T>(capacity);

	return ret;
}

template<typename T>
void clear_stack(stack<T>* s) { FUNC

	clear_vector(&s->contents);
}

template<typename T>
void stack_push(stack<T>* s, T value) { FUNC

	vector_push(&s->contents, value);
}

template<typename T>
T stack_pop(stack<T>* s) { FUNC

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
T* stack_top(stack<T>* s) { FUNC

	if(s->contents.size > 0) {

		return vector_back(&s->contents);
	}

	LOG_FATAL("Trying to get top of empty stack!");
	return NULL;
}

template<typename T>
bool stack_empty(stack<T>* s) { FUNC
	return vector_empty(&s->contents);
}
