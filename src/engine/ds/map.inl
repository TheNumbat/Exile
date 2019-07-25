
#pragma once
#include "map.h"

template<typename K, typename V>
map<K,V> map<K,V>::make(u32 capacity) { 
	
	return map<K,V>::make(capacity, CURRENT_ALLOC());
}

template<typename K, typename V>
map<K,V> map<K,V>::make(u32 capacity, allocator* a) { 
	
	map<K,V> ret;

	capacity = last_pow_two(capacity) == capacity ? capacity : next_pow_two(capacity);

	ret.alloc 	 = a;
	ret.contents = vector<map_element<K,V>>::make(capacity, ret.alloc);

	return ret;
}

template<typename K, typename V>
void map<K,V>::destroy() { 
	
	contents.destroy();

	size  = 0;
	alloc = null;
}

template<typename K, typename V>
void map<K,V>::clear() { 
	
	FORVECCAP(it, contents) {
		ELEMENT_CLEAR_OCCUPIED(*it);
	}

	contents.clear();
	size = 0;
	max_probe = 0;
}

template<typename K, typename V>
void map<K,V>::grow_rehash() { 	
	
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
void map<K,V>::trim_rehash() { 

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

template<typename K, typename V>
V* map<K,V>::get_or_insert_blank(K key) {

	V* ret = try_get(key);
	if(ret) return ret;
	return insert(key, V());
}

template<typename K, typename V>
V* map<K,V>::insert(K key, V value, bool grow_if_needed) { 
	
	if(size >= contents.capacity * MAP_MAX_LOAD_FACTOR) {

		if(grow_if_needed) {
			grow_rehash(); // this is super expensive, avoid at all costs
		} else {
			LOG_DEBUG_ASSERT(!"Map needs to grow, but not allowed!");
			return null;
		}
	}

	map_element<K,V> ele;

	ELEMENT_SET_HASH_BUCKET(ele, hash(key) & (contents.capacity - 1));
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
V* map<K,V>::insert_if_unique(K key, V value, bool grow_if_needed) { 
	
	V* result = try_get(key);
	
	if(!result) {
		
		return insert(key, value, grow_if_needed);
	}
	
	return result;
}

template<typename K, typename V>
V* map<K,V>::get(K key) { 

	V* result = try_get(key);
	LOG_ASSERT(result != null);

	return result;
}

template<typename K, typename V>
V* map<K,V>::try_get(K key) { 	// can return null

	if (size == 0) {
		return null;
	}

	u32 hash_bucket;

	hash_bucket = hash(key) & (contents.capacity - 1);

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

template<typename K, typename V>
bool map<K,V>::try_erase(K key) { 
	
	u32 hash_bucket;

	hash_bucket = hash(key) & (contents.capacity - 1);

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

template<typename K, typename V>
void map<K,V>::erase(K key) { 

	LOG_ASSERT(try_erase(key));
}
