
inline void _pop_alloc() { PROF
	this_thread_data.alloc_stack.pop();
}

inline void _push_alloc(allocator* a) { PROF
	this_thread_data.alloc_stack.push(a);
}

inline allocator* _current_alloc() { PROF
	allocator* ret = *this_thread_data.alloc_stack.top();
	return ret;
}

CALLBACK void* platform_allocate(u64 bytes, allocator* this_, code_context context) { 

	platform_allocator* this__ = (platform_allocator*)this_;

	void* mem = this__->platform_allocate(bytes);

	LOG_DEBUG_ASSERT(mem != null);

#ifdef LOG_ALLOCS
	if(!this_->suppress_messages) {
		global_log->msgf(string_literal("allocating % bytes to % with platform alloc \"%\""), log_level::alloc, context, bytes, mem, this_->name);
	}
#endif

	return mem;
}

CALLBACK void platform_free(void* mem, allocator* this_, code_context context) { 

	platform_allocator* this__ = (platform_allocator*)this_;

	LOG_DEBUG_ASSERT(mem != null);

#ifdef LOG_ALLOCS
	if(!this_->suppress_messages) {
		global_log->msgf(string_literal("freeing % with platform alloc \"%\""), log_level::alloc, context, mem, this_->name);
	}
#endif

	this__->platform_free(mem);
}

CALLBACK void* platform_reallocate(void* mem, u64 bytes, allocator* this_, code_context context) { 

	platform_allocator* this__ = (platform_allocator*)this_;

	LOG_DEBUG_ASSERT(mem != null);

#ifdef LOG_ALLOCS
	if(!this_->suppress_messages) {
		global_log->msgf(string_literal("reallocating % with to size % platform alloc \"%\""), log_level::alloc, context, mem, bytes, this_->name);
	}
#endif

	void* ret = this__->platform_reallocate(mem, bytes);

	LOG_DEBUG_ASSERT(ret != null);

	return ret;
}

inline platform_allocator make_platform_allocator(string name, code_context context) { PROF
	return np_make_platform_allocator(name, context);
}

inline platform_allocator np_make_platform_allocator(string name, code_context context) { 

	platform_allocator ret;
	
	ret.platform_allocate 	= global_api->platform_heap_alloc;
	ret.platform_free 		= global_api->platform_heap_free;
	ret.platform_reallocate = global_api->platform_heap_realloc;
	ret.context  			= context;
	ret.name				= name;

	ret.allocate_.set(FPTR(platform_allocate));
	ret.free_.set(FPTR(platform_free));
	ret.reallocate_.set(FPTR(platform_reallocate));

	return ret;
}

CALLBACK void* arena_allocate(u64 bytes, allocator* this_, code_context context) { PROF
		
	arena_allocator* this__ = (arena_allocator*)this_;

	void* mem = null;

	if(bytes <= this__->size - this__->used) {

		mem = (void*)((u8*)this__->memory + this__->used);

		this__->used += bytes;
	} else {

		LOG_ERR_F("Failed to allocate % bytes in allocator %:%", bytes, this__->context.file, this__->context.line);
	}

#ifdef LOG_ALLOCS
	if(!this__->suppress_messages) {
		global_log->msgf(string_literal("allocating % bytes (used:%/%) to % with arena alloc \"%\""), log_level::alloc, context, bytes, this__->used, this__->size, mem, this__->name);
	}
#endif

	return mem;
}

CALLBACK void arena_free(void*, allocator*, code_context context) {PROF}

CALLBACK void* arena_reallocate(void* mem, u64 bytes, allocator* this_, code_context context) { PROF

	return arena_allocate(bytes, this_, context);
}

void arena_reset(arena_allocator* a, code_context context) { PROF

#ifdef LOG_ALLOCS
	if(!a->suppress_messages) {
		global_log->msgf(string_literal("reseting arena \"%\""), log_level::alloc, context, a->name);
	}
#endif

	a->used = 0;

#ifdef ZERO_ARENA
	memset(a->memory, a->size, 0);
#endif
}

inline void arena_destroy(arena_allocator* a, code_context context) { PROF

#ifdef LOG_ALLOCS
	if(!a->suppress_messages) {
		global_log->msgf(string_literal("destroying arena \"%\""), log_level::alloc, context, a->name);
	}
#endif

	LOG_DEBUG_ASSERT(a->memory != null);
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
	ret.allocate_ 	= src.allocate_;
	ret.free_		= src.free_;
	ret.reallocate_ = src.reallocate_;
	ret.name 	  	= name;
	ret.suppress_messages = src.suppress_messages;

#ifdef LOG_ALLOCS
	if(!ret.suppress_messages) {
		global_log->msgf(string_literal("creating arena \"%\" size % copied from arena \"%\""), log_level::alloc, context, name, ret.size, src.name);
	}
#endif
	
	if(ret.size > 0) {
		ret.memory = ret.backing->allocate_(ret.size, ret.backing, context);
		memcpy(src.memory, ret.memory, ret.size);
	}

	return ret;
}

inline arena_allocator make_arena_allocator_from_context(string name, u64 size, bool suppress, code_context context) { PROF

	arena_allocator ret;

	ret.size 	  	= size;
	ret.context   	= context;
	ret.backing   	= CURRENT_ALLOC();
	ret.name 	  	= name;
	ret.suppress_messages = suppress;

	ret.allocate_.set(FPTR(arena_allocate));
	ret.free_.set(FPTR(arena_free));
	ret.reallocate_.set(FPTR(arena_reallocate));

#ifdef LOG_ALLOCS
	if(!ret.suppress_messages) {
		global_log->msgf(string_literal("creating arena \"%\" size %"), log_level::alloc, context, name, size);
	}
#endif
	
	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing, context);
	}

	return ret;
}

inline arena_allocator make_arena_allocator(string name, u64 size, allocator* backing, bool suppress, code_context context) { PROF

	arena_allocator ret;

	ret.size 	  	= size;
	ret.context   	= context;
	ret.backing   	= backing;
	ret.name 	  	= name;
	ret.suppress_messages = suppress;

	ret.allocate_.set(FPTR(arena_allocate));
	ret.free_.set(FPTR(arena_free));
	ret.reallocate_.set(FPTR(arena_reallocate));

#ifdef LOG_ALLOCS
	if(!ret.suppress_messages) {
		global_log->msgf(string_literal("creating arena \"%\" size %"), log_level::alloc, context, name, size);
	}
#endif

	if(size > 0) {
		ret.memory   = ret.backing->allocate_(size, ret.backing, context);
	}

	return ret;
}

void _memcpy(void* source, void* dest, u64 size) { PROF

	u8* csource = (u8*)source;
	u8* cdest   = (u8*)dest;

	for(u64 i = 0; i < size; i++) {
		cdest[i] = csource[i];
	}
}

void _memset(void* mem, u64 size, u8 val) { PROF

	u8* cmem = (u8*)mem;

	while(size--) {
		*cmem++ = val;
	}
}
