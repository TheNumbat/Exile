
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

	void* mem = this_->platform_allocate(bytes);

#ifdef _DEBUG
	if(!this_->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("allocating %u bytes to %p from %s:%u with platform alloc \"%s\""), log_alloc, CONTEXT, bytes, mem, context.file.c_str, context.line, this_->name.c_str);
	}
#endif

	return mem;
}

inline void platform_free(void* mem, void* this_data, code_context context) {

	platform_allocator* this_ = (platform_allocator*)this_data;

#ifdef _DEBUG
	if(!this_->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("freeing %p from %s:%u with platform alloc \"%s\""), log_alloc, CONTEXT, mem, context.file.c_str, context.line, this_->name.c_str);
	}
#endif

	this_->platform_free(mem);
}

inline platform_allocator make_platform_allocator(string name, code_context context) {

	platform_allocator ret;
	
	ret.platform_allocate 	= global_state->api->platform_heap_alloc;
	ret.platform_free 		= global_state->api->platform_heap_free;
	ret.context  			= context;
	ret.allocate_ 			= &platform_allocate;
	ret.free_ 				= &platform_free;
	ret.name				= name;

	return ret;
}

inline void* arena_allocate(u64 bytes, void* this_data, code_context context) {
		
	arena_allocator* this_ = (arena_allocator*)this_data;

	void* mem = NULL;

	if(bytes <= this_->size - this_->used) {

		mem = (void*)((u8*)this_->memory + this_->used);

		this_->used += bytes;
	} else {

		LOG_ERR_F("Failed to allocate %u bytes in allocator from %s:%u", bytes, this_->context.file.c_str, this_->context.line);
	}

#ifdef _DEBUG
	if(!this_->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("allocating %u bytes (used:%u/%u) to %p from %s:%u with arena alloc \"%s\""), log_alloc, CONTEXT, bytes, this_->used, this_->size, mem, context.file.c_str, context.line, this_->name.c_str);
	}
#endif

	return mem;
}

inline void arena_free(void*, void*, code_context context) {}

inline void arena_reset(arena_allocator* a, code_context context) {

#ifdef _DEBUG
	if(!a->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("reseting arena \"%s\""), log_alloc, CONTEXT, a->name.c_str);
	}
#endif

	a->used = 0;

#ifdef ZERO_ARENA
	memset(a->memory, a->size, 0);
#endif
}

inline void arena_destroy(arena_allocator* a, code_context context) {

#ifdef _DEBUG
	if(!a->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("destroying arena \"%s\""), log_alloc, CONTEXT, a->name.c_str);
	}
#endif


	if(a->memory) {
		if(!a->suppress_messages) LOG_PUSH_CONTEXT(a->name);
		a->backing->free_(a->memory, a->backing, CONTEXT);
		if (!a->suppress_messages) LOG_POP_CONTEXT();
	}
}

inline arena_allocator make_arena_allocator_from_context(string name, u64 size, bool suppress, code_context context) {

	arena_allocator ret;

	ret.size 	  = size;
	ret.context   = context;
	ret.backing   = CURRENT_ALLOC();
	ret.allocate_ = &arena_allocate;
	ret.free_ 	  = &arena_free;
	ret.name 	  = name;
	ret.suppress_messages = suppress;

#ifdef _DEBUG
	if(!ret.suppress_messages) {
		logger_msgf(&global_state->log, string_literal("creating arena \"%s\" size %u"), log_alloc, CONTEXT, name.c_str, size);
		LOG_PUSH_CONTEXT(name);
	}
#endif
	
	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing, CONTEXT);
	}

#ifdef _DEBUG
	if (!ret.suppress_messages) {
		LOG_POP_CONTEXT();
	}
#endif

	return ret;
}

inline arena_allocator make_arena_allocator(string name, u64 size, allocator* backing, bool suppress, code_context context) {

	arena_allocator ret;

	ret.size 	  = size;
	ret.context   = context;
	ret.backing   = backing;
	ret.allocate_ = &arena_allocate;
	ret.free_ 	  = &arena_free;
	ret.name 	  = name;
	ret.suppress_messages = suppress;

#ifdef _DEBUG
	if(!ret.suppress_messages) {
		logger_msgf(&global_state->log, string_literal("creating arena \"%s\" size %u"), log_alloc, CONTEXT, name.c_str, size);
		LOG_PUSH_CONTEXT(name);
	}
#endif

	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing, CONTEXT);
	}

#ifdef _DEBUG
	if (!ret.suppress_messages) {
		LOG_POP_CONTEXT();
	}
#endif

	return ret;
}

#define memcpy(s, d, i) _memcpy(s, d, i)
void _memcpy(void* source, void* dest, u64 size) {

	u8* csource = (u8*)source;
	u8* cdest   = (u8*)dest;

	for(u64 i = 0; i < size; i++) {
		cdest[i] = csource[i];
	}
}

void memset(void* mem, u64 size, u8 val) {

	u8* cmem = (u8*)mem;

	while(size--) {
		*cmem++ = val;
	}
}
