
#pragma once

inline u32 hash(u32 key) { 
    key = (key ^ 61) ^ (key >> 16);
    key = key + (key << 3);
    key = key ^ (key >> 4);
    key = key * 0x27d4eb2d;
    key = key ^ (key >> 15);
    return key;
}

inline u32 hash(u64 key) { 
	key = (~key) + (key << 21);
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8);
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4);
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key >> 32;
}

inline u32 hash(std::thread::id id) {
	static std::hash<std::thread::id> h;
	return h(id);
}


inline u32 hash(u8 key) { 
	return hash((u32)key);
}

inline u32 hash(u16 key) { 
	return hash((u32)key);
}


inline u32 hash(i8 key) { 
	return hash(*(u8*)&key);
}

inline u32 hash(i16 key) { 
	return hash(*(u16*)&key);
}

inline u32 hash(i32 key) { 
	return hash(*(u32*)&key);
}


template<typename T>
inline u32 hash(T* key) { 
	return hash((u64)(uptr)key);
}

template<typename A>
inline u32 hash(astring<A> str) {
    u32 h = 5381;
    for(u32 i = 0; i < str.len; i++)
        h = ((h << 5) + h) ^ str.c_str[i];
    return h;
}
