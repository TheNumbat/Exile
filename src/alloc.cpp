
void allocator::destroy() { PROF
	if(name.c_str) {
		free_(name.c_str, this, CONTEXT);
	}
}

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

CALLBACK void* platform_allocate(u64 bytes, allocator* this_, code_context context) { PROF

	platform_allocator* this__ = (platform_allocator*)this_;

	void* mem = this__->platform_allocate(bytes);

	LOG_DEBUG_ASSERT(mem != null);

#ifdef LOG_ALLOCS
	if(!this_->suppress_messages) {
		global_log->msgf(string::literal("allocating % bytes to % with platform alloc \"%\""), log_level::alloc, context, bytes, mem, this_->name);
	}
#endif

	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::allocate;
		m.context = context;
		m.allocate.to = mem;
		m.allocate.bytes = bytes;
		m.allocate.alloc = this_;

		POST_MSG(m);
	}

	return mem;
}

CALLBACK void platform_free(void* mem, allocator* this_, code_context context) { PROF

	platform_allocator* this__ = (platform_allocator*)this_;

	LOG_DEBUG_ASSERT(mem != null);

#ifdef LOG_ALLOCS
	if(!this_->suppress_messages) {
		global_log->msgf(string::literal("freeing % with platform alloc \"%\""), log_level::alloc, context, mem, this_->name);
	}
#endif

	this__->platform_free(mem);

	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::free;
		m.context = context;
		m.free.from = mem;
		m.free.alloc = this_;

		POST_MSG(m);
	}
}

CALLBACK void* platform_reallocate(void* mem, u64, u64 bytes, allocator* this_, code_context context) { PROF

	platform_allocator* this__ = (platform_allocator*)this_;

	LOG_DEBUG_ASSERT(mem != null);

#ifdef LOG_ALLOCS
	if(!this_->suppress_messages) {
		global_log->msgf(string::literal("reallocating % with to size % platform alloc \"%\""), log_level::alloc, context, mem, bytes, this_->name);
	}
#endif

	void* ret = this__->platform_reallocate(mem, bytes);

	LOG_DEBUG_ASSERT(ret != null);

	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::reallocate;
		m.context = context;
		m.reallocate.bytes = bytes;
		m.reallocate.to = ret;
		m.reallocate.from = mem;
		m.reallocate.alloc = this_;

		POST_MSG(m);
	}

	return ret;
}

inline platform_allocator make_platform_allocator(string name, code_context context) { PROF

	platform_allocator ret;
	
	ret.platform_allocate 	= global_api->platform_heap_alloc;
	ret.platform_free 		= global_api->platform_heap_free;
	ret.platform_reallocate = global_api->platform_heap_realloc;
	ret.context  			= context;
	
	ret.allocate_.set(FPTR(platform_allocate));
	ret.free_.set(FPTR(platform_free));
	ret.reallocate_.set(FPTR(platform_reallocate));

	ret.name = string::make_copy(name, &ret);

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
		global_log->msgf(string::literal("allocating % bytes (used:%/%) to % with arena alloc \"%\""), log_level::alloc, context, bytes, this__->used, this__->size, mem, this__->name);
	}
#endif

	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::allocate;
		m.context = context;
		m.allocate.to = mem;
		m.allocate.bytes = bytes;
		m.allocate.alloc = this_;

		POST_MSG(m);
	}

	return mem;
}

CALLBACK void arena_free(void*, allocator*, code_context context) {}

CALLBACK void* arena_reallocate(void* mem, u64 sz, u64 bytes, allocator* this_, code_context context) { PROF

	void* ret = arena_allocate(bytes, this_, context);
	memcpy(mem, ret, sz);
	return ret;
}

void arena_reset(arena_allocator* a, code_context context) { PROF

#ifdef LOG_ALLOCS
	if(!a->suppress_messages) {
		global_log->msgf(string::literal("reseting arena \"%\""), log_level::alloc, context, a->name);
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
		global_log->msgf(string::literal("destroying arena \"%\""), log_level::alloc, context, a->name);
	}
#endif

	LOG_DEBUG_ASSERT(a->memory != null);
	if(a->memory) {
		a->backing->free_(a->memory, a->backing, context);
	}
}

arena_allocator make_arena_allocator(string name, u64 size, allocator* backing, bool suppress, code_context context) { PROF

	arena_allocator ret;

	size += name.cap;

	ret.size 	  	= size;
	ret.context   	= context;
	ret.backing   	= backing;
	ret.suppress_messages = suppress;

	ret.allocate_.set(FPTR(arena_allocate));
	ret.free_.set(FPTR(arena_free));
	ret.reallocate_.set(FPTR(arena_reallocate));

#ifdef LOG_ALLOCS
	if(!ret.suppress_messages) {
		global_log->msgf(string::literal("creating arena \"%\" size %"), log_level::alloc, context, name, size);
	}
#endif
	
	ret.memory = ret.backing->allocate_(size, ret.backing, context);
	ret.name = string::make_copy(name, &ret);

	return ret;
}

CALLBACK void* pool_allocate(u64 bytes, allocator* this_, code_context context) {

	pool_allocator* this__ = (pool_allocator*)this_;

	if(bytes > this__->page_size) {
		LOG_ERR_F("Requesting allocation of % bytes (page size %) in pool allocator %", bytes, this__->page_size, this__->name);
		return null;
	}

	void* mem = null;
	pool_page* page = this__->current;
	if(bytes > this__->page_size - page->used) {

		page->next = (pool_page*)this__->backing->allocate_(sizeof(pool_page) + this__->page_size, this__->backing, context);
		page = this__->current = page->next;

#ifdef LOG_ALLOCS
		if(!this__->suppress_messages) {
			global_log->msgf(string::literal("adding page of size % to pool alloc \"%\""), log_level::alloc, context, this__->page_size, this__->name);
		}
#endif
	}

	mem = (void*)((u8*)page + sizeof(pool_page) + page->used);
	page->used += bytes;

	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::allocate;
		m.context = context;
		m.allocate.to = mem;
		m.allocate.bytes = bytes;
		m.allocate.alloc = this_;

		POST_MSG(m);
	}

	return mem;
}

CALLBACK void* pool_reallocate(void* mem, u64 sz, u64 bytes, allocator* this_, code_context context) {

	void* ret = pool_allocate(bytes, this_, context);
	memcpy(mem, ret, sz);
	return ret;
}

CALLBACK void  pool_free(void*, allocator*, code_context) {}

void pool_destroy(pool_allocator* a, code_context context) {

#ifdef LOG_ALLOCS
	if(!a->suppress_messages) {
		global_log->msgf(string::literal("destroying pool \"%\""), log_level::alloc, context, a->name);
	}
#endif

	LOG_DEBUG_ASSERT(a->head != null);

	pool_page* cursor = a->head;
	while(cursor) {

		pool_page* next = cursor->next;
		a->backing->free_(cursor, a->backing, context);
		cursor = next;
	}

	a->name.destroy(a->backing);
}

pool_allocator make_pool_allocator(string name, u64 page_size, allocator* backing, bool suppress, code_context context) {

	pool_allocator ret;

	ret.page_size 	= page_size;
	ret.context   	= context;
	ret.backing   	= backing;
	ret.suppress_messages = suppress;

	ret.allocate_.set(FPTR(pool_allocate));
	ret.free_.set(FPTR(pool_free));
	ret.reallocate_.set(FPTR(pool_reallocate));

#ifdef LOG_ALLOCS
	if(!ret.suppress_messages) {
		global_log->msgf(string::literal("creating pool \"%\" size %"), log_level::alloc, context, name, page_size);
	}
#endif
	
	ret.head = (pool_page*)ret.backing->allocate_(sizeof(pool_page) + page_size, ret.backing, context);
	ret.current = ret.head;
	ret.name = string::make_copy(name, backing);

	ret.head->used = 0;
	ret.head->next = 0;

	return ret;
}

void _memcpy(void* source, void* dest, u64 size) { PROF

#ifdef _MSC_VER
	__movsb((u8*)dest, (u8*)source, size);
#else

	// TODO(max): LUL this is dumb
	u8* csource = (u8*)source;
	u8* cdest   = (u8*)dest;

	for(u64 i = 0; i < size; i++) {
		cdest[i] = csource[i];
	}
#endif
}

void _memset(void* mem, u64 size, u8 val) { PROF

	u8* cmem = (u8*)mem;

	while(size--) {
		*cmem++ = val;
	}
}
