
#pragma once

template<typename K>
using Hash = u32 (K);

template<typename K, typename V>
struct norefl map_element {
    K key;
    V value;
    u32 bucket() {return _bucket >> 1;}
    u32 valid() {return _bucket & 1;}
private: u32 _bucket; // low bit set if valid
};

template<typename K, typename V, Hash<K> H = hash, typename A = Mdefault>
struct map {

    vec<map_element<K,V>, A> data;
    u32 size = 0, probe = 0;
};

template<typename K, typename V> 
struct Type_Info<map_element<K,V>> {
	static constexpr char name[] = "map_element";
	static constexpr usize size = sizeof(map_element<K,V>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _key[] = "key";
    static constexpr char _value[] = "value";
    static constexpr char _bucket[] = "_bucket";
	using members = Type_List<Record_Field<K,offset_of(&map_element<K,V>::key),_key>,
                              Record_Field<V,offset_of(&map_element<K,V>::value),_value>,
                              Record_Field<u32,offset_of(&map_element<K,V>::_bucket),_bucket>>;
};

template<typename K, typename V, Hash<K> H, typename A> 
struct Type_Info<map<K,V,H,A>> {
	static constexpr char name[] = "map";
	static constexpr usize size = sizeof(map<K,V,H,A>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _data[] = "data";
    static constexpr char _size[] = "size";
    static constexpr char _probe[] = "probe";
	using members = Type_List<Record_Field<vec<map_element<K,V>, A>,offset_of(&map<K,V,H,A>::data),_data>,
                              Record_Field<u32,offset_of(&map<K,V,H,A>::size),_size>,
                              Record_Field<u32,offset_of(&map<K,V,H,A>::probe),_probe>>;
};
