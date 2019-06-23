
#include "alloc.h"
#include "../log.h"
#include "../math.h"
#include "../dbg.h"

u32 hash(allocator a) { 

	return hash(a.name());
}

bool operator==(allocator l, allocator r) { 

	return l.context == r.context;
}

string allocator::name() { 
	return string::from_c_str(c_name);
}

CALLBACK void* platform_allocate(u64 bytes, u64 align, allocator* this_, code_context context) { 

	void* mem = global_api->heap_alloc(bytes);

	LOG_DEBUG_ASSERT(mem != null);

#ifdef PROFILE
	if(this_thread_data.startup) return mem;

	dbg_msg m;
	m.type = dbg_msg_type::allocate;
	m.context = context;
	m.allocate.to = mem;
	m.allocate.bytes = this_->track_sizes ? bytes : 0;
	m.allocate.alloc = *this_;
	POST_MSG(m);
#endif 

	return mem;
}

CALLBACK void platform_free(void* mem, u64 sz, allocator* this_, code_context context) { 

	LOG_DEBUG_ASSERT(mem != null);

	global_api->heap_free(mem);

#ifdef PROFILE
	if(this_thread_data.startup) return;

	dbg_msg m;
	m.type = dbg_msg_type::free;
	m.context = context;
	m.free.from = mem;
	m.free.alloc = *this_;
	m.free.bytes = this_->track_sizes ? sz : 0;
	POST_MSG(m);
#endif 
}

CALLBACK void* platform_reallocate(void* mem, u64 sz, u64 bytes, u64 align, allocator* this_, code_context context) { 

	LOG_DEBUG_ASSERT(mem != null);

	void* ret = global_api->heap_realloc(mem, bytes);

	LOG_DEBUG_ASSERT(ret != null);

#ifdef PROFILE
	if(this_thread_data.startup) return ret;

	dbg_msg m;
	m.type = dbg_msg_type::reallocate;
	m.context = context;
	m.reallocate.to_bytes = this_->track_sizes ? bytes : 0;
	m.reallocate.from_bytes = this_->track_sizes ? sz : 0;
	m.reallocate.to = ret;
	m.reallocate.from = mem;
	m.reallocate.alloc = *this_;
	POST_MSG(m);
#endif 

	return ret;
}

platform_allocator make_platform_allocator(string name, code_context context) { 

	platform_allocator ret;
	
	ret.context = context;
	
	ret.allocate_.set(FPTR(platform_allocate));
	ret.free_.set(FPTR(platform_free));
	ret.reallocate_.set(FPTR(platform_reallocate));

	_memcpy(name.c_str, ret.c_name, name.len);

	return ret;
}

CALLBACK void* arena_allocate(u64 bytes, u64 align, allocator* this_, code_context context) { 
		
	arena_allocator* this__ = (arena_allocator*)this_;

	void* mem = null;
	
	u64 pad = align ? mod(this__->used + (uptr)this__->memory, align) : 0;
		pad = (pad ? align - pad : 0);

	if(bytes + pad <= this__->size - this__->used) {

		this__->used += pad;
		mem = (void*)((u8*)this__->memory + this__->used);
		this__->used += bytes;
	} else {

		LOG_ERR_F("Failed to allocate % bytes in allocator %:%"_, bytes, this__->context.file(), this__->context.line);
	}

	return mem;
}

CALLBACK void arena_free(void*, u64, allocator*, code_context context) {}

CALLBACK void* arena_reallocate(void* mem, u64 sz, u64 bytes, u64 align, allocator* this_, code_context context) { 

	void* ret = arena_allocate(bytes, align, this_, context);
	_memcpy(mem, ret, sz);
	return ret;
}

void arena_reset(arena_allocator* a, code_context context) { 

	a->used = 0;

#ifdef ZERO_ARENA
	_memset(a->memory, a->size, 0);
#endif
}

void arena_destroy(arena_allocator* a, code_context context) { 

	LOG_DEBUG_ASSERT(a->memory != null);
	if(a->memory) {
		a->backing->free_(a->memory, a->size, a->backing, context);
	}
}

arena_allocator make_arena_allocator(string name, u64 size, allocator* backing, code_context context) { 

	arena_allocator ret;

	size += name.cap;

	ret.size 	  	= size;
	ret.context   	= context;
	ret.backing   	= backing;

	ret.allocate_.set(FPTR(arena_allocate));
	ret.free_.set(FPTR(arena_free));
	ret.reallocate_.set(FPTR(arena_reallocate));

	ret.memory = ret.backing->allocate_(size, 0, ret.backing, context);
	
	LOG_DEBUG_ASSERT(name.len < ALLOC_NAME_LEN);
	_memcpy(name.c_str, ret.c_name, name.len);

	return ret;
}

CALLBACK void* pool_allocate(u64 bytes, u64 align, allocator* this_, code_context context) {

	pool_allocator* this__ = (pool_allocator*)this_;

	if(bytes > this__->page_size) {
		LOG_ERR_F("Requesting allocation of % bytes (page size %) in pool allocator %"_, bytes, this__->page_size, this__->name());
		return null;
	}

	void* mem = null;
	pool_page* page = this__->current;

	u64 pad = align ? mod(page->used + (uptr)page + sizeof(pool_page), align) : 0;
		pad = (pad ? align - pad : 0);

	if(bytes + pad > this__->page_size - page->used) {

		page->next = (pool_page*)this__->backing->allocate_(sizeof(pool_page) + this__->page_size, 0, this__->backing, context);
		page = this__->current = page->next;
		
		pad = align ? mod(page->used, align) : 0;
		pad = (pad ? align - pad : 0);		
	}

	page->used += pad;
	mem = (void*)((u8*)page + sizeof(pool_page) + page->used);
	page->used += bytes;

	return mem;
}

CALLBACK void* pool_reallocate(void* mem, u64 sz, u64 bytes, u64 align, allocator* this_, code_context context) {

	void* ret = pool_allocate(bytes, align, this_, context);
	_memcpy(mem, ret, sz);
	return ret;
}

CALLBACK void pool_free(void*, u64, allocator*, code_context) {}

void pool_destroy(pool_allocator* a, code_context context) {

	LOG_DEBUG_ASSERT(a->head != null);

	pool_page* cursor = a->head;
	while(cursor) {

		pool_page* next = cursor->next;
		a->backing->free_(cursor, a->page_size + sizeof(pool_page), a->backing, context);
		cursor = next;
	}
}

pool_allocator make_pool_allocator(string name, u64 page_size, allocator* backing, bool suppress, code_context context) {

	pool_allocator ret;

	ret.page_size 	= page_size;
	ret.context   	= context;
	ret.backing   	= backing;

	ret.allocate_.set(FPTR(pool_allocate));
	ret.free_.set(FPTR(pool_free));
	ret.reallocate_.set(FPTR(pool_reallocate));
	
	ret.head = (pool_page*)ret.backing->allocate_(sizeof(pool_page) + page_size, 0, ret.backing, context);
	ret.current = ret.head;
	_memcpy(name.c_str, ret.c_name, name.len);

	ret.head->used = 0;
	ret.head->next = 0;

	return ret;
}

void _memcpy(void* source, void* dest, u64 size) {

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

void _memset(void* mem, u64 size, u8 val) { 

#ifdef _MSC_VER

	u64 val64 = val;
	val64 |= val64 << 8;
	val64 |= val64 << 16;
	val64 |= val64 << 32;

	__stosq((u64*)mem, val64, size / 8);
	__stosb((u8*)mem + (size & ~7), val, size & 7);

#else
	u8* cmem = (u8*)mem;

	while(size--) {
		*cmem++ = val;
	}
#endif
}
