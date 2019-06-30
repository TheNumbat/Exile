
#pragma once

#include "vector.h"
#include "../math.h"

// don't store addresses of elements in a map - at all. Pls.

// if the map is filled beyond this load factor, it will try to grow
const f32 MAP_MAX_LOAD_FACTOR = 0.9f;

// from Thomas Wang, http://burtleburtle.net/bob/hash/integer.html
inline u32 hash(u32 key) { 
    key = (key ^ 61) ^ (key >> 16);
    key = key + (key << 3);
    key = key ^ (key >> 4);
    key = key * 0x27d4eb2d;
    key = key ^ (key >> 15);
    return key;
}
inline u32 hash(u64 key) { 
	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key >> 32;
}
inline u32 hash(void* key) { 
	return hash(*(u64*)&key);
}
inline u32 hash(i32 key) { 
	return hash(*(u32*)&key);
}
inline u32 hash(u8 key) { 
	return hash((u32)key);
}
inline u32 hash(u16 key) { 
	return hash((u32)key);
}
// map foreach
#define FORMAP(it,m) for(auto it = (m).contents.memory; it != (m).contents.memory + (m).contents.capacity; it++) if(ELEMENT_OCCUPIED(*it))

#define ELEMENT_OCCUPIED_FLAG 				(1<<31)
#define ELEMENT_OCCUPIED(me) 				((me).occupied_and_bucket & ELEMENT_OCCUPIED_FLAG)
#define ELEMENT_SET_OCCUPIED(me) 			((me).occupied_and_bucket |= ELEMENT_OCCUPIED_FLAG)
#define ELEMENT_CLEAR_OCCUPIED(me) 			((me).occupied_and_bucket &= ~ELEMENT_OCCUPIED_FLAG)
#define ELEMENT_HASH_BUCKET(me) 			((me).occupied_and_bucket & ~ELEMENT_OCCUPIED_FLAG)
#define ELEMENT_SET_HASH_BUCKET(me, val) 	((me).occupied_and_bucket = val | ELEMENT_OCCUPIED(me))

template<typename K, typename V>
struct map_element {
	K key;
	V value;
	// TODO(max): test if storing hashes in a separate array performs better

	// NOTE(max): highest bit of hash_bucket signifies occupation; bucket index can be 31 bits
	// 			  use macros to get hash_bucket/manipulate values
	u32 occupied_and_bucket = 0;
};

template<typename K, typename V, u32(hash_func)(K) = hash>
struct map {
	vector<map_element<K, V>> contents;
	u32 size	 		= 0;				// always a power of two so mod is only a bit-and
	u32 max_probe		= 0;
	allocator* alloc 	= null;

///////////////////////////////////////////////////////////////////////////////

	static map<K,V,hash_func> make(u32 capacity = 16);
	static map<K,V,hash_func> make(u32 capacity, allocator* a);
	
	void destroy();
	
	V* insert(K key, V value, bool grow_if_needed = true);
	V* insert_if_unique(K key, V value, bool grow_if_needed = true);
	bool try_erase(K key);
	void erase(K key);
	void clear();

	V* get(K key);
	V* try_get(K key);
	V* get_or_insert_blank(K key);

	// this is expensive, avoid at all costs. Try to create maps with enough capacity in the first place.
	// it will allocate & free another map-sized block for copying from the map's allocator
	// this is called from map_insert if the map needs to grow...be wary
	void grow_rehash();
	void trim_rehash();
};
