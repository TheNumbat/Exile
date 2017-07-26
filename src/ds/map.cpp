
inline u32 hash_u32(u32 key) { PROF
    key = (key ^ 61) ^ (key >> 16);
    key = key + (key << 3);
    key = key ^ (key >> 4);
    key = key * 0x27d4eb2d;
    key = key ^ (key >> 15);
    
    return key;
}

inline u32 hash_u64(u64 key) { PROF
	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return (u32)key;			// TODO(max): uh, is this OK? probably not.
}

template<typename K, typename V>
map<K,V> make_map(i32 capacity, u32 (*hash)(K)) { PROF
	map<K,V> ret = make_map<K,V>(capacity, CURRENT_ALLOC(), hash);
	
	return ret;
}

template<typename K, typename V>
map<K,V> make_map(i32 capacity, allocator* a, u32 (*hash)(K)) { PROF
	map<K,V> ret;

	capacity = (i32)ceilf(capacity / MAP_MAX_LOAD_FACTOR);

	ret.alloc 	 = a;
	ret.contents = make_vector<map_element<K,V>>(capacity, ret.alloc);
	if(!hash) {
		LOG_DEBUG_ASSERT(sizeof(K) == sizeof(u32));
		ret.use_u32hash = true;
	} else {
		ret.hash = hash;
	}

	
	return ret;
}

template<typename K, typename V>
void destroy_map(map<K,V>* m) { PROF
	
	destroy_vector(&m->contents);

	m->size  = 0;
	m->alloc = NULL;
	m->hash  = NULL;
}

template<typename K, typename V>
void map_clear(map<K,V>* m) { PROF
	
	FORVEC(m->contents,
		it->occupied = false;
	)
	clear_vector(&m->contents);
	m->size = 0;
	m->max_probe = 0;
}

template<typename K, typename V>
void map_grow_rehash(map<K,V>* m) { PROF	
	
	vector<map_element<K,V>> temp = make_vector_copy(m->contents);

	vector_grow(&m->contents, false);

	m->size = 0;
	m->max_probe = 0;

	for(u32 i = 0; i < temp.capacity; i++) {
		if(vector_get(&temp, i)->occupied == true) {
			map_insert(m, vector_get(&temp, i)->key, vector_get(&temp, i)->value);
		}
	}

	destroy_vector(&temp);
}

template<typename K, typename V> 
void map_trim_rehash(map<K,V>* m) { PROF

	vector<map_element<K,V>> temp = make_vector_copy(m->contents);

	vector_resize(&m->contents, m->size, false);

	m->size = 0;
	m->max_probe = 0;
	
	for(u32 i = 0; i < temp.capacity; i++) {
		if(vector_get(&temp, i)->occupied == true) {
			map_insert(m, vector_get(&temp, i)->key, vector_get(&temp, i)->value);
		}
	}

	destroy_vector(&temp);
}

template<typename K, typename V>
V* map_insert(map<K,V>* m, K key, V value, bool grow_if_needed) { PROF
	
	if(m->size >= m->contents.capacity * MAP_MAX_LOAD_FACTOR) {

		if(grow_if_needed) {
			map_grow_rehash(m); // this is super expensive, avoid at all costs
		} else {
			LOG_DEBUG_ASSERT(!"Map needs to grow, but not allowed!");
			return NULL;
		}
	}

	map_element<K,V> ele;

	if(m->use_u32hash) {
		ele.hash_bucket = mod(hash_u32(*((u32*)&key)), m->contents.capacity);
	} else {
		ele.hash_bucket = mod((*m->hash)(key), m->contents.capacity);
	}
	ele.key 		= key;
	ele.value 		= value;
	ele.occupied 	= true;

	// robin hood open addressing

	u32 index = ele.hash_bucket;
	u32 probe_length = 0;
	map_element<K,V>* placed_adr = NULL;
	for(;;) {
		if(vector_get(&m->contents, index)->occupied) {

			i32 occupied_probe_length = index - vector_get(&m->contents, index)->hash_bucket;
			if(occupied_probe_length < 0) {
				occupied_probe_length += m->contents.capacity;
			}

			if((u32)occupied_probe_length < probe_length) {

				map_element<K,V>* to_swap = vector_get(&m->contents, index);
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
			if (index == m->contents.capacity) {
				index = 0;
			}

			if(probe_length > m->max_probe) {
				m->max_probe = probe_length;
			}
		} else {
			*vector_get(&m->contents, index) = ele;
			m->size++;

			if(placed_adr) {
				return &placed_adr->value;
			}
			return &(vector_get(&m->contents, index)->value);
		}
	}
}

template<typename K, typename V>
V* map_insert_if_unique(map<K,V>* m, K key, V value, bool grow_if_needed) { PROF
	
	V* result = map_try_get(m, key);
	
	if(!result) {
		
		return map_insert(m, key, value, grow_if_needed);
	}

	
	return result;
}

template<typename K, typename V>
V* map_get(map<K,V>* m, K key) { PROF

	V* result = map_try_get(m, key);
	LOG_ASSERT(result != NULL);

	return result;
}

template<typename K, typename V>
V* map_try_get(map<K,V>* m, K key) { PROF	// can return NULL

	if (m->size == 0) {
		return NULL;
	}

	u32 hash_bucket;

	if(m->use_u32hash) {
		hash_bucket = mod(hash_u32(*((u32*)&key)), m->contents.capacity);
	} else {
		hash_bucket = mod((*m->hash)(key), m->contents.capacity);
	}

	u32 index = hash_bucket;
	u32 probe_length = 0;
	for(;;) {

		if(vector_get(&m->contents, index)->key == key) {
			return &(vector_get(&m->contents, index)->value);
		}

		probe_length++;
		if(probe_length > m->max_probe) {
			return NULL;
		}

		index++;
		if (index == m->contents.capacity) {
			index = 0;
		}
	}
}

template<typename K, typename V>
void map_erase(map<K,V>* m, K key) { PROF
	
	u32 hash_bucket;

	if(m->use_u32hash) {
		hash_bucket = mod(hash_u32(*((u32*)&key)), m->contents.capacity);
	} else {
		hash_bucket = mod((*m->hash)(key), m->contents.capacity);
	}

	u32 index = hash_bucket;
	u32 probe_length = 0;
	for(;;) {

		if(vector_get(&m->contents, index)->key == key) {
			vector_get(&m->contents, index)->occupied = false;
			m->size--;
		}

		probe_length++;
		if(probe_length > m->max_probe) {
			return;
		}

		index++;
		if (index == m->contents.capacity) {
			index = 0;
		}
	}
}
