
#pragma once

struct allocator {
	void* (*allocate_)(u64 bytes, void* this_data, code_context context)  = NULL;
	void  (*free_)(void* mem, void* this_data, code_context context)	  = NULL;
	code_context context;
	bool suppress_messages = false;
	string name;
};

#define PUSH_ALLOC(a)	_push_alloc(a);
#define POP_ALLOC() 	_pop_alloc()
#define CURRENT_ALLOC()	_current_alloc()

#define KILOBYTES(m) (m*1024)
#define MEGABYTES(m) (KILOBYTES(m*1024))
#define GIGABYTES(m) (MEGABYTES(m*1024))

inline allocator* _current_alloc();
inline void _pop_alloc();
inline void _push_alloc(allocator* a);
inline allocator* _current_alloc();

struct platform_allocator : public allocator {
	void* (*platform_allocate)(u64 bytes) = NULL;
	void  (*platform_free)(void* mem)	  = NULL;
};

void* platform_allocate(u64 bytes, void* this_data, code_context context);
void platform_free(void* mem, void* this_data, code_context context);

#define MAKE_PLATFORM_ALLOCATOR(n) make_platform_allocator(string_literal(n), CONTEXT)
platform_allocator make_platform_allocator(string name, code_context context);

struct arena_allocator : public allocator {
	allocator* backing;
	void* memory 	= NULL;
	u64 used		= 0;
	u64 size		= 0;
};

void* arena_allocate(u64 bytes, void* this_data, code_context context);
void arena_free(void*, void*, code_context); // does nothing

#define DESTROY_ARENA(a) arena_destroy(a, CONTEXT) 
void arena_destroy(arena_allocator* a, code_context context);

#define	RESET_ARENA(a) arena_reset(a, CONTEXT) 
void arena_reset(arena_allocator* a, code_context context);

#define MAKE_ARENA_FROM_CONTEXT(n, size, s) make_arena_allocator_from_context(string_literal(n), size, s, CONTEXT) 
arena_allocator make_arena_allocator_from_context(string name, u64 size, bool suppress, code_context context);

#define MAKE_ARENA(n, size, a, s) make_arena_allocator(string_literal(n), size, a, s, CONTEXT) 
arena_allocator make_arena_allocator(string name, u64 size, allocator* backing, bool suppress, code_context context);

// USE THESE TO USE CONTEXT SYSTEM - ALWAYS USE THEM UNELSS YOU HAVE YOUR OWN ALLOCATOR STRUCT
#define malloc(b) ((*CURRENT_ALLOC()->allocate_)(b, CURRENT_ALLOC(), CONTEXT)) 
#define free(m) ((*CURRENT_ALLOC()->free_)(m, CURRENT_ALLOC(), CONTEXT)) 

void memcpy(void* source, void* dest, u64 size);
void memset(void* mem, u64 size, u8 val);
