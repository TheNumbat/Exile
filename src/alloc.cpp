
inline void _pop_alloc() { FUNC
	stack_pop(&this_thread_data.alloc_stack);
}

inline void _push_alloc(allocator* a) { FUNC
	stack_push(&this_thread_data.alloc_stack,a);
}

inline allocator* _current_alloc() { FUNC
	allocator* ret = *stack_top(&this_thread_data.alloc_stack);
	return ret;
}

inline void* platform_allocate(u64 bytes, void* this_data, code_context context) { FUNC

	platform_allocator* this_ = (platform_allocator*)this_data;

	void* mem = this_->platform_allocate(bytes);

#ifdef _DEBUG
	if(!this_->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("allocating % bytes to % with platform alloc \"%\""), log_level::alloc, context, bytes, mem, this_->name);
	}
#endif

	return mem;
}

inline void platform_free(void* mem, void* this_data, code_context context) { FUNC

	platform_allocator* this_ = (platform_allocator*)this_data;

#ifdef _DEBUG
	if(!this_->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("freeing % with platform alloc \"%\""), log_level::alloc, context, mem, this_->name);
	}
#endif

	this_->platform_free(mem);
}

inline platform_allocator make_platform_allocator(string name, code_context context) { FUNC

	platform_allocator ret;
	
	ret.platform_allocate 	= global_state->api->platform_heap_alloc;
	ret.platform_free 		= global_state->api->platform_heap_free;
	ret.context  			= context;
	ret.allocate_ 			= &platform_allocate;
	ret.free_ 				= &platform_free;
	ret.name				= name;

	return ret;
}

inline void* arena_allocate(u64 bytes, void* this_data, code_context context) { FUNC
		
	arena_allocator* this_ = (arena_allocator*)this_data;

	void* mem = NULL;

	if(bytes <= this_->size - this_->used) {

		mem = (void*)((u8*)this_->memory + this_->used);

		this_->used += bytes;
	} else {

		LOG_ERR_F("Failed to allocate % bytes in allocator %:%", bytes, this_->context.file, this_->context.line);
	}

#ifdef _DEBUG
	if(!this_->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("allocating % bytes (used:%/%) to % with arena alloc \"%\""), log_level::alloc, context, bytes, this_->used, this_->size, mem, this_->name);
	}
#endif

	return mem;
}

inline void arena_free(void*, void*, code_context context) { FUNC}

inline void arena_reset(arena_allocator* a, code_context context) { FUNC

#ifdef _DEBUG
	if(!a->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("reseting arena \"%\""), log_level::alloc, context, a->name);
	}
#endif

	a->used = 0;

#ifdef ZERO_ARENA
	memset(a->memory, a->size, 0);
#endif
}

inline void arena_destroy(arena_allocator* a, code_context context) { FUNC

#ifdef _DEBUG
	if(!a->suppress_messages) {
		logger_msgf(&global_state->log, string_literal("destroying arena \"%\""), log_level::alloc, context, a->name);
	}
#endif


	if(a->memory) {
		a->backing->free_(a->memory, a->backing, context);
	}
}

arena_allocator arena_copy(string name, allocator* backing, arena_allocator src, code_context context) {

	arena_allocator ret;

	ret.size = src.size;
	ret.context = context;
	if(backing) {
		ret.backing = backing;
	} else {
		ret.backing = src.backing;
	}
	ret.allocate_ = src.allocate_;
	ret.free_	  = src.free_;
	ret.name 	  = name;
	ret.suppress_messages = src.suppress_messages;

#ifdef _DEBUG
	if(!ret.suppress_messages) {
		logger_msgf(&global_state->log, string_literal("creating arena \"%\" size % copied from arena \"%\""), log_level::alloc, context, name, ret.size, src.name);
	}
#endif
	
	if(ret.size > 0) {
		ret.memory = ret.backing->allocate_(ret.size, ret.backing, context);
		memcpy(src.memory, ret.memory, ret.size);
	}

	return ret;
}

inline arena_allocator make_arena_allocator_from_context(string name, u64 size, bool suppress, code_context context) { FUNC

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
		logger_msgf(&global_state->log, string_literal("creating arena \"%\" size %"), log_level::alloc, context, name, size);
	}
#endif
	
	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing, context);
	}

	return ret;
}

inline arena_allocator make_arena_allocator(string name, u64 size, allocator* backing, bool suppress, code_context context) { FUNC

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
		logger_msgf(&global_state->log, string_literal("creating arena \"%\" size %"), log_level::alloc, context, name, size);
	}
#endif

	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing, context);
	}

	return ret;
}

void _memcpy(void* source, void* dest, u64 size) { FUNC

	u8* csource = (u8*)source;
	u8* cdest   = (u8*)dest;

	for(u64 i = 0; i < size; i++) {
		cdest[i] = csource[i];
	}
}

void _memset(void* mem, u64 size, u8 val) { FUNC

	u8* cmem = (u8*)mem;

	while(size--) {
		*cmem++ = val;
	}
}
