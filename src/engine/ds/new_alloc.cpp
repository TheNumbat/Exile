
DEF_ALLOC(basic);
DEF_ARENA(buf, basic, 1024);

void test_allocators() {

	int* mem = (int*)buf::allocate(1024, CONTEXT);

	for(i32 i = 0; i < 256; i++) {
		mem[i] = i;
	}

	buf::_free(mem, CONTEXT);
	buf::destroy(CONTEXT);
}

template<typename name>
void* mem_alloc<name>::allocate(u64 size, code_context ctx) {
	LOG_INFO_F("%b a %", size, TYPENAME(name));
	return global_api->heap_alloc(size);
}

template<typename name>
void* mem_alloc<name>::reallocate(void* mem, u64 size, code_context ctx) {
	LOG_INFO_F("%b r %", size, TYPENAME(name));
	return global_api->heap_realloc(mem, size);
}

template<typename name>
void mem_alloc<name>::_free(void* mem, code_context ctx) {
	LOG_INFO_F("f %", TYPENAME(name));
	global_api->heap_free(mem);
}


template<typename name, typename backing, u64 total_size>
void* mem_arena<name, backing, total_size>::allocate(u64 size, code_context ctx) {
	LOG_INFO_F("%b a %", size, TYPENAME(name));

	if(!memory) {
		memory = backing::allocate(total_size, ctx);
	}
	if(used + size > total_size) {
		return null;
	}
	used += size;
	return (u8*)memory + used - size;
}

template<typename name, typename backing, u64 total_size>
void* mem_arena<name, backing, total_size>::reallocate(void* mem, u64 size, code_context ctx) {
	LOG_INFO_F("%b r %", size, TYPENAME(name));
	return allocate(size, ctx);
}

template<typename name, typename backing, u64 total_size>
void mem_arena<name, backing, total_size>::_free(void* mem, code_context ctx) {
	LOG_INFO_F("f %", TYPENAME(name));
}

template<typename name, typename backing, u64 total_size>
void mem_arena<name, backing, total_size>::destroy(code_context ctx) {
	if(memory) {
		backing::_free(memory, ctx);
		memory = null;
	}
}


template<typename name, typename backing, u64 page_size>
void* mem_pool<name, backing, page_size>::allocate(u64 size, code_context ctx) {
	LOG_INFO_F("%b a %", size, TYPENAME(name));
	return backing::allocate(size, ctx);
}

template<typename name, typename backing, u64 page_size>
void* mem_pool<name, backing, page_size>::reallocate(void* mem, u64 size, code_context ctx) {
	LOG_INFO_F("%b r %", size, TYPENAME(name));
	return backing::reallocate(mem, size, ctx);
}

template<typename name, typename backing, u64 page_size>
void mem_pool<name, backing, page_size>::_free(void* mem, code_context ctx) {
	LOG_INFO_F("f %", TYPENAME(name));
	backing::_free(mem, ctx);
}
