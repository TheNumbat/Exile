
#pragma once

#define ALLOC_NAME_LEN 25

struct allocator {
	func_ptr<void*,u64,u64,allocator*,code_context>				allocate_;
	func_ptr<void*,void*,u64,u64,u64,allocator*,code_context> 	reallocate_;
	func_ptr<void,void*,u64,allocator*,code_context> 			free_;

	code_context context;
	bool suppress_messages = false;

	char c_name[ALLOC_NAME_LEN] = {};
	string name();
};

#define KILOBYTES(m) (m*1024)
#define MEGABYTES(m) (KILOBYTES(m*1024))
#define GIGABYTES(m) (MEGABYTES(m*1024))

#define PUSH_ALLOC(a)	this_thread_data.alloc_stack.push(a);
#define POP_ALLOC() 	this_thread_data.alloc_stack.pop();
#define CURRENT_ALLOC()	(*this_thread_data.alloc_stack.top())

#define MAKE_PLATFORM_ALLOCATOR(n) make_platform_allocator(n##_, CONTEXT)

#define MAKE_ARENA(n, size, a, s) make_arena_allocator(n, size, a, s, CONTEXT) 
#define MAKE_ARENA_FROM_CONTEXT(n, size, s) make_arena_allocator(n, size, CURRENT_ALLOC(), s, CONTEXT) 
#define DESTROY_ARENA(a) arena_destroy(a, CONTEXT);
#define	RESET_ARENA(a) arena_reset(a, CONTEXT);

#define MAKE_POOL(n, size, a, s) make_pool_allocator(n, size, a, s, CONTEXT) 
#define MAKE_POOL_FROM_CONTEXT(n, size, s) make_pool_allocator(n, size, CURRENT_ALLOC(), s, CONTEXT)
#define DESTROY_POOL(a) pool_destroy(a, CONTEXT);

#define malloc(sz) 	((CURRENT_ALLOC()->allocate_)(sz, 0, CURRENT_ALLOC(), CONTEXT)) 
#define free(m,sz) 	((CURRENT_ALLOC()->free_)((void*)m, sz, CURRENT_ALLOC(), CONTEXT));

#define NEW(T)    (new ((T*)malloc(sizeof(T))) T)
#define NEWA(T,c) (new ((T*)malloc(c * sizeof(T))) T[c])

#define memcpy(s,d,i) _memcpy(s,d,i);
#define memset(m,s,v) _memset(m,s,v);

struct platform_allocator : public allocator {};

platform_allocator make_platform_allocator(string name, code_context context);

CALLBACK void* platform_allocate(u64 bytes, u64 align, allocator* this_, code_context context);
CALLBACK void  platform_free(void* mem, u64 sz, allocator* this_, code_context context);
CALLBACK void* platform_reallocate(void* mem, u64 sz, u64 bytes, u64 align, allocator* this_, code_context context);

struct arena_allocator : public allocator {
	allocator* backing 	= null;
	void* memory 		= null;
	u64 used			= 0;
	u64 size			= 0;
};

arena_allocator make_arena_allocator(string name, u64 size, allocator* backing, bool suppress, code_context context);
void arena_destroy(arena_allocator* a, code_context context);
void arena_reset(arena_allocator* a, code_context context);

CALLBACK void* arena_allocate(u64 bytes, u64 align, allocator* this_, code_context context);
CALLBACK void* arena_reallocate(void* mem, u64 sz, u64 bytes, u64 align, allocator* this_, code_context context); // same as allocate, can't free from arena
CALLBACK void  arena_free(void*, u64, allocator*, code_context); // does nothing

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

pool_allocator make_pool_allocator(string name, u64 page_size, allocator* backing, bool suppress, code_context context);
void pool_destroy(pool_allocator* a, code_context context);

CALLBACK void* pool_allocate(u64 bytes, u64 align, allocator* this_, code_context context);
CALLBACK void* pool_reallocate(void* mem, u64 sz, u64 bytes, u64 align, allocator* this_, code_context context); // same as allocate, can't free from arena
CALLBACK void  pool_free(void*, u64, allocator*, code_context); // does nothing

void _memcpy(void* source, void* dest, u64 size);
void _memset(void* mem, u64 size, u8 val);
void _memcpy_ctx(void* source, void* dest, u64 size);

bool operator==(allocator l, allocator r);
inline u32 hash(allocator a);
