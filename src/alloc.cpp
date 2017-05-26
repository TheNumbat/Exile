
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

void* platform_allocate(u64 bytes, void* this_data) {

	platform_allocator* this_ = (platform_allocator*)this_data;

	return this_->platform_allocate(bytes);
}

void platform_free(void* mem, void* this_data) {

	platform_allocator* this_ = (platform_allocator*)this_data;

	return this_->platform_free(mem);
}

void platform_destroy(void*) {}

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

void memcpy(void* source, void* dest, u64 size) {

	char* csource = (char*)source;
	char* cdest   = (char*)dest;

	for(int i = 0; i < size; i++)
		cdest[i] = csource[i];
}
