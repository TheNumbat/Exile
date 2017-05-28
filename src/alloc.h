
#pragma once

// forward declarations ... thanks c++
template<typename T> struct stack;
template<typename K, typename V> struct map;

struct allocator {
	void* (*allocate_)(u64 bytes, void* this_data) = NULL;
	void  (*free_)(void* mem, void* this_data)	   = NULL;
	code_context context;
};

#define PUSH_ALLOC(a)			_push_alloc(a);
#define POP_ALLOC() 			_pop_alloc()
#define CURRENT_ALLOC()			_current_alloc()

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

inline void* platform_allocate(u64 bytes, void* this_data);
inline void platform_free(void* mem, void* this_data);

#define MAKE_PLATFORM_ALLOCATOR() make_platform_allocator(CONTEXT)
inline platform_allocator make_platform_allocator(code_context context);

struct arena_allocator : public allocator {
	allocator* backing;
	void* memory 	= NULL;
	u64 used		= 0;
	u64 size		= 0;
};

inline void* arena_allocate(u64 bytes, void* this_data);
inline void arena_free(void*, void*); // does nothing

#define DESTROY_ARENA(a) arena_destroy(a)
inline void arena_destroy(arena_allocator* a);

#define MAKE_ARENA_FROM_CONTEXT(size) make_arena_allocator_from_context(size, CONTEXT)
inline arena_allocator make_arena_allocator_from_context(u64 size, code_context context);

#define MAKE_ARENA(size, a) make_arena_allocator(size, a, CONTEXT)
inline arena_allocator make_arena_allocator(u64 size, allocator* backing, code_context context);

// USE THESE TO USE CONTEXT SYSTEM - ALWAYS USE THEM UNELSS YOU HAVE YOUR OWN ALLOCATOR STRUCT
#define malloc(b) ((*CURRENT_ALLOC()->allocate_)(b, CURRENT_ALLOC()))
#define free(m) ((*CURRENT_ALLOC()->free_)(m, CURRENT_ALLOC()))

void memcpy(void* source, void* dest, u64 size);
