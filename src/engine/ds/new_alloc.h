
#pragma once

template<typename name>
struct mem_alloc {
	static void* allocate(u64 size, code_context ctx);
	static void* reallocate(void* mem, u64 size, code_context ctx);
	static void  _free(void* mem, code_context ctx);
};

template<typename name, typename backing, u64 size>
struct mem_arena {
	static void* allocate(u64 size, code_context ctx);
	static void* reallocate(void* mem, u64 size, code_context ctx);
	static void  _free(void* mem, code_context ctx);
	static void destroy(code_context ctx);
private:
	static void* memory;
	static u64 used;
};

template<typename name, typename backing, u64 page_size>
struct mem_pool {
	static void* allocate(u64 size, code_context ctx);
	static void* reallocate(void* mem, u64 size, code_context ctx);
	static void  _free(void* mem, code_context ctx);
};

#define DEF_ALLOC(name) \
	enum name##_ {}; \
	typedef mem_alloc<name##_> name;

#define DEF_ARENA(name, backing, size) \
	enum name##_ {}; \
	typedef mem_arena<name##_, backing, size> name; \
	void* name::memory = null; \
	u64 name::used = 0;

void _memcpy(void* source, void* dest, u64 size);
void _memset(void* mem, u64 size, u8 val);
void _memcpy_ctx(void* source, void* dest, u64 size);

void test_allocators();

