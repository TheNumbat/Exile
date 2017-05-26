
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
void _pop_alloc();
void _push_alloc(allocator* a);
allocator* _current_alloc();

struct platform_allocator : public allocator {
	void* (*platform_allocate)(u64 bytes) = NULL;
	void  (*platform_free)(void* mem)	  = NULL;
};

void* platform_allocate(u64 bytes, void* this_data);
void platform_free(void* mem, void* this_data);
void platform_destroy(void*);

#define MAKE_PLATFORM_ALLOCATOR() make_platform_allocator(CONTEXT)
inline platform_allocator make_platform_allocator(code_context context);

struct arena_allocator : public allocator {
	allocator* backing;
	void* memory 	= NULL;
	u64 used		= 0;
	u64 size		= 0;
};

void* arena_allocate(u64 bytes, void* this_data);
void arena_free(void*, void*);
void arena_destroy(void* this_data);

#define MAKE_ARENA_ALLOCATOR_FROM_CONTEXT(size) make_arena_allocator_from_context(size, CONTEXT)
inline allocator make_arena_allocator_from_context(u64 size, code_context context);

#define MAKE_ARENA_ALLOCATOR(size, a) make_arena_allocator(size, a, CONTEXT)
inline arena_allocator make_arena_allocator(u64 size, allocator* backing, code_context context);

// USE THESE TO USE CONTEXT SYSTEM - ALWAYS USE THEM UNELSS YOU HAVE YOUR OWN ALLOCATOR STRUCT
#define _malloc(b) ((*CURRENT_ALLOC()->allocate_)(b, CURRENT_ALLOC()))
#define _free(m) ((*CURRENT_ALLOC()->free_)(m, CURRENT_ALLOC()))

void memcpy(void* source, void* dest, u64 size);
