
#pragma once

// forward declarations ... thanks c++
template<typename T> struct stack;
template<typename K, typename V> struct map;

struct allocator {
	void* (*allocate_)(u64 bytes, void* this_data) = NULL;
	void  (*free_)(void* mem, void* this_data)	   = NULL;
	void  (*destroy)(void* this_data)			   = NULL;
	code_context context;
};

static map<platform_thread_id, stack<allocator*>>* global_alloc_contexts = NULL;
static platform_mutex* global_alloc_contexts_mutex 						 = NULL;

#define PUSH_ALLOC(a)			_push_alloc(a);
#define POP_ALLOC() 			_pop_alloc()
#define CURRENT_ALLOC()			_current_alloc()

#define KILOBYTES(m) (m*1024)
#define MEGABYTES(m) (KILOBYTES(m*1024))
#define GIGABYTES(m) (MEGABYTES(m*1024))

allocator* _current_alloc();

#include "stack.h"
#include "map.h"

void _pop_alloc() {
	global_platform_api->platform_aquire_mutex(global_alloc_contexts_mutex, -1);
	(*CURRENT_ALLOC()->destroy)(CURRENT_ALLOC()); 
	stack_pop(&map_get(global_alloc_contexts, global_platform_api->platform_this_thread_id()));
	global_platform_api->platform_release_mutex(global_alloc_contexts_mutex);
}

void _push_alloc(allocator* a) {
	global_platform_api->platform_aquire_mutex(global_alloc_contexts_mutex, -1);
	stack_push(&map_get(global_alloc_contexts, global_platform_api->platform_this_thread_id()),a);
	global_platform_api->platform_release_mutex(global_alloc_contexts_mutex);
}

allocator* _current_alloc() {
	global_platform_api->platform_aquire_mutex(global_alloc_contexts_mutex, -1);
	allocator* ret = stack_top(&map_get(global_alloc_contexts, global_platform_api->platform_this_thread_id()));
	global_platform_api->platform_release_mutex(global_alloc_contexts_mutex);
	return ret;
}

struct platform_allocator : public allocator {
	void* (*platform_allocate)(u64 bytes) = NULL;
	void  (*platform_free)(void* mem)	  = NULL;
};

void* platform_allocate(u64 bytes, void* this_data) {

	platform_allocator* this_ = (platform_allocator*)this_data;

	return this_->platform_allocate(bytes);
}

void platform_free(void* mem, void* this_data) {

	platform_allocator* this_ = (platform_allocator*)this_data;

	return this_->platform_free(mem);
}

void platform_destroy(void*) {}

#define MAKE_PLATFORM_ALLOCATOR() make_platform_allocator(CONTEXT)
inline platform_allocator make_platform_allocator(code_context context) {

	platform_allocator ret;
	
	ret.platform_allocate = global_platform_api->platform_heap_alloc;
	ret.platform_free	  = global_platform_api->platform_heap_free;
	ret.context  		  = context;
	ret.allocate_ 		  = &platform_allocate;
	ret.free_ 			  = &platform_free;
	ret.destroy 		  = &platform_destroy;

	return ret;
}

struct arena_allocator : public allocator {
	allocator* backing;
	void* memory 	= NULL;
	u64 used		= 0;
	u64 size		= 0;
};

void* arena_allocate(u64 bytes, void* this_data) {
		
	arena_allocator* this_ = (arena_allocator*)this_data;

	if(bytes <= this_->size - this_->used) {

		void* ret = (void*)((u8*)this_->memory + this_->used);

		this_->used += bytes;

		return ret;
	}

	return NULL;
}

void arena_free(void*, void*) {}

void arena_destroy(void* this_data) {

	arena_allocator* this_ = (arena_allocator*)this_data;

	if(this_->memory) {

		this_->backing->free_(this_->memory, this_->backing);
	}
}

#define MAKE_ARENA_ALLOCATOR_FROM_CONTEXT(size) make_arena_allocator_from_context(size, CONTEXT)
inline allocator make_arena_allocator_from_context(u64 size, code_context context) {

	arena_allocator ret;

	ret.size 	  = size;
	ret.context   = context;
	ret.backing   = CURRENT_ALLOC();
	ret.allocate_ = &arena_allocate;
	ret.free_ 	  = &arena_free;
	ret.destroy   = &arena_destroy;
	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing);
	}

	return ret;
}

#define MAKE_ARENA_ALLOCATOR(size, a) make_arena_allocator(size, a, CONTEXT)
inline arena_allocator make_arena_allocator(u64 size, allocator* backing, code_context context) {

	arena_allocator ret;

	ret.size 	  = size;
	ret.context   = context;
	ret.backing   = backing;
	ret.allocate_ = &arena_allocate;
	ret.free_ 	  = &arena_free;
	ret.destroy   = &arena_destroy;
	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing);
	}

	return ret;
}

// USE THESE TO USE CONTEXT SYSTEM - ALWAYS USE THEM UNELSS YOU HAVE YOUR OWN ALLOCATOR STRUCT
#define _malloc(b) ((*CURRENT_ALLOC()->allocate_)(b, CURRENT_ALLOC()))
#define _free(m) ((*CURRENT_ALLOC()->free_)(m, CURRENT_ALLOC()))

void memcpy(void* source, void* dest, u64 size) {

	char* csource = (char*)source;
	char* cdest   = (char*)dest;

	for(int i = 0; i < size; i++)
		cdest[i] = csource[i];
}