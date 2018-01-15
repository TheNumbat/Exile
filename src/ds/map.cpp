
CALLBACK u32 hash_u32(u32 key) { PROF
    key = (key ^ 61) ^ (key >> 16);
    key = key + (key << 3);
    key = key ^ (key >> 4);
    key = key * 0x27d4eb2d;
    key = key ^ (key >> 15);
    
    return key;
}

CALLBACK u32 hash_u64(u64 key) { PROF
	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key >> 32;
}

CALLBACK u32 hash_ptr(void* key) { PROF

	return hash_u64((u64)key);
}

template<typename K, typename V>
map<K,V> map<K,V>::make(u32 capacity, _FPTR* hash) { PROF
	
	return map<K,V>::make(capacity, CURRENT_ALLOC(), hash);
}

template<typename K, typename V>
map<K,V> map<K,V>::make(u32 capacity, allocator* a, _FPTR* hash) { PROF
	map<K,V> ret;

	capacity = last_pow_two(capacity) == capacity ? capacity : next_pow_two(capacity);

	ret.alloc 	 = a;
	ret.contents = vector<map_element<K,V>>::make(capacity, ret.alloc);
	if(!hash) {
		LOG_DEBUG_ASSERT(sizeof(K) == sizeof(u32));
		ret.use_u32hash = true;
	} else {
		ret.hash.set(hash);
	}

	return ret;
}

template<typename K, typename V>
void map<K,V>::destroy() { PROF
	
	contents.destroy();

	size  = 0;
	alloc = null;
}

template<typename K, typename V>
void map<K,V>::clear() { PROF
	
	FORVEC(it, contents) {
		ELEMENT_CLEAR_OCCUPIED(*it);
	}

	contents.clear();
	size = 0;
	max_probe = 0;
}

template<typename K, typename V>
void map<K,V>::grow_rehash() { PROF	
	
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

template<typename K, typename V> 
void map<K,V>::trim_rehash() { PROF

	vector<map_element<K,V>> temp = make_vector_copy(contents);

	contents.resize(size, false);

	size = 0;
	max_probe = 0;
	
	FORVECCAP(it, temp) {
		if(ELEMENT_OCCUPIED(*it)) {
			insert(it->key, it->value);
		}
	}

	destroy_vector(&temp);
}

template<typename K, typename V>
V* map<K,V>::insert(K key, V value, bool grow_if_needed) { PROF
	
	if(size >= contents.capacity * MAP_MAX_LOAD_FACTOR) {

		if(grow_if_needed) {
			grow_rehash(); // this is super expensive, avoid at all costs
		} else {
			LOG_DEBUG_ASSERT(!"Map needs to grow, but not allowed!");
			return null;
		}
	}

	map_element<K,V> ele;

	if(use_u32hash) {
		ELEMENT_SET_HASH_BUCKET(ele, hash_u32(*((u32*)&key)) & (contents.capacity - 1));
	} else {
		ELEMENT_SET_HASH_BUCKET(ele, hash(key) & (contents.capacity - 1));
	}
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

template<typename K, typename V>
V* map<K,V>::insert_if_unique(K key, V value, bool grow_if_needed) { PROF
	
	V* result = try_get(key);
	
	if(!result) {
		
		return insert(key, value, grow_if_needed);
	}
	
	return result;
}

template<typename K, typename V>
V* map<K,V>::get(K key) { PROF

	V* result = try_get(key);
	LOG_ASSERT(result != null);

	return result;
}

template<typename K, typename V>
V* map<K,V>::try_get(K key) { PROF	// can return null

	if (size == 0) {
		return null;
	}

	u32 hash_bucket;

	if(use_u32hash) {
		hash_bucket = hash_u32(*((u32*)&key)) & (contents.capacity - 1);
	} else {
		hash_bucket = hash(key) & (contents.capacity - 1);
	}	

	u32 index = hash_bucket;
	u32 probe_length = 0;
	for(;;) {

		if(contents.get(index)->key == key) {
			return &(contents.get(index)->value);
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

template<typename K, typename V>
void map<K,V>::erase(K key) { PROF
	
	u32 hash_bucket;

	if(use_u32hash) {
		hash_bucket = hash_u32(*((u32*)&key)) & (contents.capacity - 1);
	} else {
		hash_bucket = hash(key) & (contents.capacity - 1);
	}	

	u32 index = hash_bucket;
	u32 probe_length = 0;
	for(;;) {

		if(contents.get(index)->key == key) {
			ELEMENT_CLEAR_OCCUPIED(*contents.get(index));
			size--;
			return;
		}

		probe_length++;
		if(probe_length > max_probe) {
			return;
		}

		index++;
		if (index == contents.capacity) {
			index = 0;
		}
	}
}
