
#pragma once

// forward declarations ... thanks c++
template<typename T> struct vector;
template<typename T> using stack = vector<T>;

struct allocator {
	void* (*allocate)(u64 bytes) = NULL;
	void  (*free)(void* mem)	 = NULL;
	code_context context;
};

stack<allocator>* global_alloc_context;

#include "stack.h"

#define ALLOCATOR(a, f) 		make_allocator(a, f, CONTEXT)
#define PUSH_ALLOC(a)			stack_push(global_alloc_context,a);
#define PUSH_ALLOC_FUNC(a, f) 	stack_push(global_alloc_context,ALLOCATOR(a, f));
#define POP_ALLOC() 			stack_pop(global_alloc_context);
#define CURRENT_ALLOC() 		stack_top(global_alloc_context)

inline allocator make_allocator(void* (*allocate)(u64 bytes), void  (*free)(void* mem), code_context context) {

	allocator ret;
	ret.allocate = allocate;
	ret.free = free;
	ret.context = context;

	return ret;
}

#define malloc(b) _malloc(b, CONTEXT)
inline void* _malloc(u64 bytes, code_context context) {

	allocator current = stack_top(global_alloc_context);

	return (*current.allocate)(bytes);
}

#define free(m) _free(m, CONTEXT)
inline void _free(void* mem, code_context context) {

	allocator current = stack_top(global_alloc_context);

	return (*current.free)(mem);
}

void memcpy(void* source, void* dest, u64 size) {

	char* csource = (char*)source;
	char* cdest   = (char*)dest;

	for(int i = 0; i < size; i++)
		cdest[i] = csource[i];
}