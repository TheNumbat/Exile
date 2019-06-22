
#include "map.h"



template<typename K, typename V, u32(*hash_func)(K) = hash>
map<K,V,hash_func> map<K,V,hash_func>::make(u32 capacity) { 
	
	return map<K,V,hash_func>::make(capacity, CURRENT_ALLOC());
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
map<K,V,hash_func> map<K,V,hash_func>::make(u32 capacity, allocator* a) { 
	
	map<K,V,hash_func> ret;

	capacity = last_pow_two(capacity) == capacity ? capacity : next_pow_two(capacity);

	ret.alloc 	 = a;
	ret.contents = vector<map_element<K,V>>::make(capacity, ret.alloc);

	return ret;
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
void map<K,V,hash_func>::destroy() { 
	
	contents.destroy();

	size  = 0;
	alloc = null;
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
void map<K,V,hash_func>::clear() { 
	
	FORVECCAP(it, contents) {
		ELEMENT_CLEAR_OCCUPIED(*it);
	}

	contents.clear();
	size = 0;
	max_probe = 0;
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
void map<K,V,hash_func>::grow_rehash() { 	
	
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
void map<K,V,hash_func>::trim_rehash() { 

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
V* map<K,V,hash_func>::get(K key) { 

	V* result = try_get(key);
	LOG_ASSERT(result != null);

	return result;
}

template<typename K, typename V, u32(*hash_func)(K) = hash>
V* map<K,V,hash_func>::try_get(K key) { 	// can return null

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
bool map<K,V,hash_func>::try_erase(K key) { 
	
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
void map<K,V,hash_func>::erase(K key) { 

	LOG_ASSERT(try_erase(key));
}
