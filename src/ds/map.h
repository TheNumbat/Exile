
#pragma once

// don't store addresses of elements in a map - at all. Pls.

const f32 MAP_MAX_LOAD_FACTOR = 0.9f;

// from Thomas Wang, http://burtleburtle.net/bob/hash/integer.html
inline u32 hash_u32(u32 key);
inline u32 hash_u64(u64 key);

template<typename K, typename V>
struct map_element {
	K key;
	V value;
	// TODO(max): test if storing hashes in a separate array performs better
	// TODO(max): use less storage than a bool to signify occupation
	bool occupied = false;
	u32 hash_bucket = 0;
};

template<typename K, typename V>
struct map {
	vector<map_element<K, V>> contents;
	u32 size	 		= 0;
	allocator* alloc 	= null;
	u32 (*hash)(K key) 	= null;
	bool use_u32hash 	= false;
	u32 max_probe		= 0;

///////////////////////////////////////////////////////////////////////////////

	static map<K,V> make(i32 capacity = 16, u32 (*hash)(K) = null);
	static map<K,V> make(i32 capacity, allocator* a, u32 (*hash)(K) = null);
	
	void destroy();
	
	V* insert(K key, V value, bool grow_if_needed = true);
	V* insert_if_unique(K key, V value, bool grow_if_needed = true);
	void erase(K key);
	void clear();

	V* get(K key);
	V* try_get(K key);

	// this is expensive, avoid at all costs. Try to create maps with enough capacity in the first place.
	// it will allocate & free another map-sized block for copying from the map's allocator
	// this is called from map_insert if the map needs to grow...be wary
	void grow_rehash();
	void trim_rehash();
};
