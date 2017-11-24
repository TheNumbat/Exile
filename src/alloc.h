
#pragma once

struct allocator {
	func_ptr<void*,u64,allocator*,code_context>				allocate_;
	func_ptr<void*,void*,u64,u64,allocator*,code_context> 	reallocate_;
	func_ptr<void,void*,allocator*,code_context> 			free_;

	void destroy();

	code_context context;
	bool suppress_messages = false;
	string name;
};

#define PUSH_ALLOC(a)	_push_alloc(a);
#define POP_ALLOC() 	_pop_alloc();
#define CURRENT_ALLOC()	_current_alloc()

#define KILOBYTES(m) (m*1024)
#define MEGABYTES(m) (KILOBYTES(m*1024))
#define GIGABYTES(m) (MEGABYTES(m*1024))

inline allocator* _current_alloc();
inline void _pop_alloc();
inline void _push_alloc(allocator* a);
inline allocator* _current_alloc();

struct platform_allocator : public allocator {
	void* (*platform_allocate)(u64 bytes) 				= null;
	void  (*platform_free)(void* mem)	  				= null;
	void* (*platform_reallocate)(void* mem, u64 bytes)	= null;
};

CALLBACK void* platform_allocate(u64 bytes, allocator* this_, code_context context);
CALLBACK void  platform_free(void* mem, allocator* this_, code_context context);
CALLBACK void* platform_reallocate(void* mem, u64, u64 bytes, allocator* this_, code_context context);

#define MAKE_PLATFORM_ALLOCATOR(n) make_platform_allocator(string::literal(n), CONTEXT)
inline platform_allocator make_platform_allocator(string name, code_context context);

struct arena_allocator : public allocator {
	allocator* backing 	= null;
	void* memory 		= null;
	u64 used			= 0;
	u64 size			= 0;
};

CALLBACK void* arena_allocate(u64 bytes, allocator* this_, code_context context);
CALLBACK void* arena_reallocate(void* mem, u64 sz, u64 bytes, allocator* this_, code_context context); // same as allocate, can't free from arena
CALLBACK void  arena_free(void*, allocator*, code_context); // does nothing

#define DESTROY_ARENA(a) arena_destroy(a, CONTEXT) 
void arena_destroy(arena_allocator* a, code_context context);

#define	RESET_ARENA(a) arena_reset(a, CONTEXT) 
void arena_reset(arena_allocator* a, code_context context);

#define MAKE_ARENA_FROM_CONTEXT(n, size, s) make_arena_allocator(n, size, CURRENT_ALLOC(), s, CONTEXT) 
#define MAKE_ARENA(n, size, a, s) make_arena_allocator(n, size, a, s, CONTEXT) 
arena_allocator make_arena_allocator(string name, u64 size, allocator* backing, bool suppress, code_context context);

struct pool_page {
	u64 used 		= 0;
	pool_page* next = null;
	// page memory is allocated directly after the header
};

// basically an arena that pushes new arenas onto a list if it needs more memory
struct pool_allocator : public allocator {

	allocator* backing 	= null;
	u64 page_size 		= 0;
	pool_page* head 	= null;
	pool_page* current 	= null; // TODO(max): do we want a current page, or should each allocation iterate the pages attempting to find one with space?
								// currently if an allocation is too big to fit on the current page, another page is allocated and any space in the 
								// last page is wasted.
};

CALLBACK void* pool_allocate(u64 bytes, allocator* this_, code_context context);
CALLBACK void* pool_reallocate(void* mem, u64 sz, u64 bytes, allocator* this_, code_context context); // same as allocate, can't free from arena
CALLBACK void  pool_free(void*, allocator*, code_context); // does nothing

#define DESTROY_POOL(a) pool_destroy(a, CONTEXT) 
void pool_destroy(pool_allocator* a, code_context context);

#define MAKE_POOL_FROM_CONTEXT(n, size, s) make_pool_allocator(n, size, CURRENT_ALLOC(), s, CONTEXT) 
#define MAKE_POOL(n, size, a, s) make_pool_allocator(n, size, a, s, CONTEXT) 
pool_allocator make_pool_allocator(string name, u64 page_size, allocator* backing, bool suppress, code_context context);

#define malloc(b) 	((CURRENT_ALLOC()->allocate_)((u64)b, CURRENT_ALLOC(), CONTEXT)) 
#define free(m) 	((CURRENT_ALLOC()->free_)((void*)m, CURRENT_ALLOC(), CONTEXT)) 

#define memcpy(s,d,i) _memcpy(s,d,i)
void _memcpy(void* source, void* dest, u64 size);
#define memset(m,s,v) _memset(m,s,v)
void _memset(void* mem, u64 size, u8 val);

#define NEW(t)    _new<t>()
#define NEWA(t,c) _new_array<t>(c)

template<typename T>
T* _new() {
	return new ((T*)malloc(sizeof(T))) T;
}

template<typename T>
T* _new_array(u64 c) {
	return new ((T*)malloc(c * sizeof(T))) T[c];
}
