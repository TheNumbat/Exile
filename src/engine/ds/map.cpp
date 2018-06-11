
inline u32 hash(u32 key) { PROF

    key = (key ^ 61) ^ (key >> 16);
    key = key + (key << 3);
    key = key ^ (key >> 4);
    key = key * 0x27d4eb2d;
    key = key ^ (key >> 15);
    
    return key;
}

inline u32 hash(u64 key) { PROF

	// return (u32)(key >> 32 ^ key);

	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key >> 32;
}

inline u32 hash(void* key) { PROF

	// NOTE(max): this better be compiled as 64 bit lol
	return hash(*(u64*)&key);
}

inline u32 hash(i32 key) { PROF

	return hash(*(u32*)&key);
}

inline u32 hash(u8 key) { PROF

	return hash((u32)key);
}

inline u32 hash(u16 key) { PROF

	return hash((u32)key);
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
map<K,V,hash_func> map<K,V,hash_func>::make(u32 capacity) { PROF
	
	return map<K,V,hash_func>::make(capacity, CURRENT_ALLOC());
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
map<K,V,hash_func> map<K,V,hash_func>::make(u32 capacity, allocator* a) { PROF
	
	map<K,V,hash_func> ret;

	capacity = last_pow_two(capacity) == capacity ? capacity : next_pow_two(capacity);

	ret.alloc 	 = a;
	ret.contents = vector<map_element<K,V>>::make(capacity, ret.alloc);

	return ret;
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
void map<K,V,hash_func>::destroy() { PROF
	
	contents.destroy();

	size  = 0;
	alloc = null;
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
void map<K,V,hash_func>::clear() { PROF
	
	FORVEC(it, contents) {
		ELEMENT_CLEAR_OCCUPIED(*it);
	}

	contents.clear();
	size = 0;
	max_probe = 0;
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
void map<K,V,hash_func>::grow_rehash() { PROF	
	
	vector<map_element<K,V>> temp = vector<map_element<K,V>>::make_copy(contents);

	contents.grow();
	contents.zero();

	size = 0;
	max_probe = 0;

	FORVECCAP(it, temp) {
		if(ELEMENT_OCCUPIED(*it)) {
			insert(it->key, it->value);
		}
	}

	temp.destroy();
}

template<typename K, typename V, u32(*hash_func)(K) = hash> 
void map<K,V,hash_func>::trim_rehash() { PROF

	vector<map_element<K,V>> temp = vector<map_element<K,V>>::make_copy(contents);

	u32 new_capacity = last_pow_two(contents.capacity) == contents.capacity ? contents.capacity : next_pow_two(contents.capacity);
	contents.resize(new_capacity);

	size = 0;
	max_probe = 0;
	
	FORVECCAP(it, temp) {
		if(ELEMENT_OCCUPIED(*it)) {
			insert(it->key, it->value);
		}
	}

	temp.destroy();
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
V* map<K,V,hash_func>::insert(K key, V value, bool grow_if_needed) { PROF
	
	if(size >= contents.capacity * MAP_MAX_LOAD_FACTOR) {

		if(grow_if_needed) {
			grow_rehash(); // this is super expensive, avoid at all costs
		} else {
			LOG_DEBUG_ASSERT(!"Map needs to grow, but not allowed!");
			return null;
		}
	}

	map_element<K,V> ele;

	ELEMENT_SET_HASH_BUCKET(ele, hash_func(key) & (contents.capacity - 1));
	ele.key 		= key;
	ele.value 		= value;
	ELEMENT_SET_OCCUPIED(ele);

	// robin hood open addressing

	u32 index = ELEMENT_HASH_BUCKET(ele);
	u32 probe_length = 0;
	map_element<K,V>* placed_adr = null;
	for(;;) {
		if(ELEMENT_OCCUPIED(*contents.get(index))) {

			i32 occupied_probe_length = index - ELEMENT_HASH_BUCKET(*contents.get(index));
			if(occupied_probe_length < 0) {
				occupied_probe_length += contents.capacity;
			}

			if((u32)occupied_probe_length < probe_length) {

				map_element<K,V>* to_swap = contents.get(index);
				if(!placed_adr) {
					placed_adr = to_swap;
				}

				map_element<K,V> temp = *to_swap;
				*to_swap = ele;
				ele = temp;

				probe_length = occupied_probe_length;
			} 

			probe_length++;
			index++;
			if (index == contents.capacity) {
				index = 0;
			}

			if(probe_length > max_probe) {
				max_probe = probe_length;
			}
		} else {
			*contents.get(index) = ele;
			size++;

			if(placed_adr) {
				return &placed_adr->value;
			}
			return &(contents.get(index)->value);
		}
	}
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
V* map<K,V,hash_func>::insert_if_unique(K key, V value, bool grow_if_needed) { PROF
	
	V* result = try_get(key);
	
	if(!result) {
		
		return insert(key, value, grow_if_needed);
	}
	
	return result;
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
V* map<K,V,hash_func>::get(K key) { PROF

	V* result = try_get(key);
	LOG_ASSERT(result != null);

	return result;
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
V* map<K,V,hash_func>::try_get(K key) { PROF	// can return null

	if (size == 0) {
		return null;
	}

	u32 hash_bucket;

	hash_bucket = hash_func(key) & (contents.capacity - 1);

	u32 index = hash_bucket;
	u32 probe_length = 0;
	for(;;) {

		map_element<K,V>* ele = contents.get(index);
		if(ELEMENT_OCCUPIED(*ele) && ele->key == key) {
			return &ele->value;
		}

		probe_length++;
		if(probe_length > max_probe) {
			return null;
		}

		index++;
		if (index == contents.capacity) {
			index = 0;
		}
	}
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
bool map<K,V,hash_func>::try_erase(K key) { PROF
	
	u32 hash_bucket;

	hash_bucket = hash_func(key) & (contents.capacity - 1);

	u32 index = hash_bucket;
	u32 probe_length = 0;
	for(;;) {

		map_element<K,V>* ele = contents.get(index);

		if(ELEMENT_OCCUPIED(*ele) && ele->key == key) {
			ELEMENT_CLEAR_OCCUPIED(*contents.get(index));
			size--;
			return true;
		}

		probe_length++;
		if(probe_length > max_probe) {
			return false;
		}

		index++;
		if (index == contents.capacity) {
			index = 0;
		}
	}
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
void map<K,V,hash_func>::erase(K key) { PROF

	LOG_ASSERT(try_erase(key));
}
