
#pragma once

enum allocator_type {
	gen,
	arena,
	pool
};

template<allocator_type type, typename name>
struct new_allocator {};

template<typename name>
struct new_allocator<gen, name> {
	static void* allocate(u64 size, code_context ctx);
	static void* reallocate(void* mem, u64 size, code_context ctx);
	static void  _free(void* mem, code_context ctx);
};

template<typename name>
struct new_allocator<arena, name> {
	static void* allocate(u64 size, code_context ctx);
	static void* reallocate(void* mem, u64 size, code_context ctx);
	static void  _free(void* mem, code_context ctx);
};

void _memcpy(void* source, void* dest, u64 size);
void _memset(void* mem, u64 size, u8 val);
void _memcpy_ctx(void* source, void* dest, u64 size);

void test_allocators();
