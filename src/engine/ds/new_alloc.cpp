
enum basic {};

void test_allocators() {

	typedef new_allocator<allocator_type::gen, basic> basic;

	int* mem = (int*)basic::allocate(1024, CONTEXT);

	for(i32 i = 0; i < 256; i++) {
		mem[i] = i;
	}

	basic::_free(mem, CONTEXT);
}

template<typename name>
void* new_allocator<gen, name>::allocate(u64 size, code_context ctx) {
	string n = TYPENAME(name);
	LOG_INFO("alloc");
	return global_api->heap_alloc(size);
}

template<typename name>
void* new_allocator<gen, name>::reallocate(void* mem, u64 size, code_context ctx) {
	LOG_INFO("realloc");
	return null;
}

template<typename name>
void new_allocator<gen, name>::_free(void* mem, code_context ctx) {
	LOG_INFO("free");
	global_api->heap_free(mem);
}
