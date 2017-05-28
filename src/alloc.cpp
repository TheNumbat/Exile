
inline void _pop_alloc() {
	global_state->api->platform_aquire_mutex(&global_state->alloc_contexts_mutex, -1);
	stack_pop(map_get(&global_state->alloc_contexts, global_state->api->platform_this_thread_id()));
	global_state->api->platform_release_mutex(&global_state->alloc_contexts_mutex);
}

inline void _push_alloc(allocator* a) {
	global_state->api->platform_aquire_mutex(&global_state->alloc_contexts_mutex, -1);
	stack_push(map_get(&global_state->alloc_contexts, global_state->api->platform_this_thread_id()),a);
	global_state->api->platform_release_mutex(&global_state->alloc_contexts_mutex);
}

inline allocator* _current_alloc() {
	global_state->api->platform_aquire_mutex(&global_state->alloc_contexts_mutex, -1);
	allocator* ret = stack_top(map_get(&global_state->alloc_contexts, global_state->api->platform_this_thread_id()));
	global_state->api->platform_release_mutex(&global_state->alloc_contexts_mutex);
	return ret;
}

inline void* platform_allocate(u64 bytes, void* this_data, code_context context) {

	platform_allocator* this_ = (platform_allocator*)this_data;

	return this_->platform_allocate(bytes);
}

inline void platform_free(void* mem, void* this_data, code_context context) {

	platform_allocator* this_ = (platform_allocator*)this_data;

	return this_->platform_free(mem);
}

inline platform_allocator make_platform_allocator(code_context context) {

	platform_allocator ret;
	
	ret.platform_allocate = global_state->api->platform_heap_alloc;
	ret.platform_free	  = global_state->api->platform_heap_free;
	ret.context  		  = context;
	ret.allocate_ 		  = &platform_allocate;
	ret.free_ 			  = &platform_free;

	return ret;
}

inline void* arena_allocate(u64 bytes, void* this_data, code_context context) {
		
	arena_allocator* this_ = (arena_allocator*)this_data;

	if(bytes <= this_->size - this_->used) {

		void* ret = (void*)((u8*)this_->memory + this_->used);

		this_->used += bytes;

		return ret;
	} else {

		LOG_ERR_F("Failed to allocate %u bytes in allocator from %s:%u", bytes, this_->context.file.c_str, this_->context.line);
	}

	return NULL;
}

inline void arena_free(void*, void*, code_context context) {}

inline void arena_destroy(arena_allocator* a, code_context context) {

	if(a->memory) {

		a->backing->free_(a->memory, a->backing, CONTEXT);
	}
}

inline arena_allocator make_arena_allocator_from_context(u64 size, code_context context) {

	arena_allocator ret;

	ret.size 	  = size;
	ret.context   = context;
	ret.backing   = CURRENT_ALLOC();
	ret.allocate_ = &arena_allocate;
	ret.free_ 	  = &arena_free;
	
	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing, CONTEXT);
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

	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing, CONTEXT);
	}

	return ret;
}

void memcpy(void* source, void* dest, u64 size) {

	u8* csource = (u8*)source;
	u8* cdest   = (u8*)dest;

	for(int i = 0; i < size; i++)
		cdest[i] = csource[i];
}
