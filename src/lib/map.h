
#pragma once

template<typename K>
using Hash = u32 (K);

template<typename K, typename V>
struct map_slot {
    K key;
    V value;

    void destroy() {
        if(!valid()) return;
        if constexpr(is_Destroy<K>()) key.destroy();
        if constexpr(is_Destroy<V>()) value.destroy();
        _bucket = 0;
    }

private:
    u32 bucket() {return _bucket >> 1;}
    bool valid() {return _bucket & 1;}
    void set_valid(bool v) {_bucket |= (u32)(!!v);}
    void set_bucket(u32 b) {_bucket = (b << 1) | (b & 1);}

    u32 _bucket = 0; // low bit set if valid

    template<typename _K, typename _V, Hash<K> _H, typename _A>
    friend struct map;
};

template<typename K, typename V, Hash<K> H = hash, typename A = Mdefault>
struct map {

    static const inline f32 max_load_factor = 0.9f;
    using slot = map_slot<K,V>;

    // capacity should always be a power of 2
    vec<slot, A> data;
    u32 size = 0, probe = 0, usable = 0;

    static map make(u32 capacity) {
        map ret = {vec<slot,A>::make(ceil_pow2(capacity)), 0, 0, 0};
        ret.data.size = ret.data.capacity;
        return ret;
    }
    static map copy(map source) {
        return {vec<slot,A>::copy(source), source.size, source.probe, source.usable};
    }
    static map take(map& source) {
        map ret = source;
        source = {{}, 0, 0, 0};
        return ret;
    }

    void destroy() {
        data.destroy();
        size = probe = usable = 0;
    }
    
    void grow() {
        vec<slot,A> next = vec<slot,A>::make(data.size ? data.size * 2 : 8);
        next.size = next.capacity;
        vec<slot,A> prev = data;
        data = next;

        size = probe = 0;
        usable = (u32)floorf(next.capacity * max_load_factor);
        for(auto& e : prev) {
            if(e.valid()) {
                insert(e.key, e.value);
                e.set_valid(false);
            }
        }
        prev.destroy();
    }
    void clear() {
        for(auto& e : data) e.set_valid(false);
        size = probe = 0;
    }

    bool empty() const {
        return size == 0;
    }
    bool full() const {
        return size == usable;
    }

    V& insert(K key, V value) {
        if(full()) grow();
        
        u32 idx = H(key) & (data.capacity - 1), distance = 0;
        slot* placement = null, new_slot;
        new_slot.key = key; new_slot.value = value;
        new_slot.set_bucket(idx);
        new_slot.set_valid(true);

        for(;;) {
            slot& here = data[idx];
            if(here.valid()) {
                i32 here_dist = idx > here.bucket() ? idx - here.bucket() : idx + data.capacity - here.bucket();
                if((u32)here_dist < distance) {
                    if(!placement) placement = &here;
                    slot swap = here;
                    here = new_slot;
                    new_slot = swap;
                    distance = here_dist;
                }
                distance++;
                idx = idx == data.capacity ? 0 : idx + 1;
                probe = _MAX(probe, distance);
            } else {
                here = new_slot;
                size++;
                if(placement) return placement->value;
                return data[idx].value;
            }
        }
    }
    V* try_get(K key) {
        if(empty()) return null;

        u32 bucket = H(key) & (data.capacity - 1);
        u32 idx = bucket, distance = 0;
        for(;;) {
            slot& s = data[idx];
            if(s.valid() && s.key == key) return &s.value;
            distance++;
            if(distance > probe) return null;
            idx = idx == data.capacity ? 0 : idx + 1;
        }
    }
    bool try_erase(K key) {
        u32 bucket = H(key) & (data.capacity - 1);
        u32 idx = bucket, distance = 0;
        for(;;) {
            slot& s = data[idx];
            if(s.valid() && s.key == key) {
                s.set_valid(false);
                size--;
                return true;
            }
            distance++;
            if(distance > probe) return false;
            idx = idx == data.capacity ? 0 : idx + 1;
        }
    }
    
    V& get_or_insert(K key, V default_value = V()) {
        V* value = try_get(key);
        return value ? value : insert(key, default_value);
    }
    V& get(K key) {
        V* value = try_get(key);
        if(!value) die("Failed to find key %!", key);
        return *value;
    }
    void erase(K key) {
        if(!try_erase(key)) die("Failed to erase key %!", key);
    }

    template<typename S>
    struct itr {
        itr(map& _m, u32 idx) : m(_m) {
            place = idx;
            while(place < m.data.capacity && !m.data[place].valid()) place++;
        }

        itr operator++(int) { 
            itr i = *this; 
            do { place++;
            } while(place < m.data.capacity && !m.data[place].valid()); 
            return i; 
        }
        itr operator++() { 
            do { place++; 
            } while(place < m.data.capacity && !m.data[place].valid()); 
            return *this; 
        }
        S& operator*() { return m.data[place]; }
        S* operator->() { return &m.data[place]; }
        bool operator==(const itr& rhs) { return &m == &rhs.m && place == rhs.place; }
        bool operator!=(const itr& rhs) { return &m != &rhs.m || place != rhs.place; }
        
        map& m;
        u32 place = 0;
    };
    typedef itr<slot> iterator;
    typedef itr<const slot> const_iterator;

    const_iterator begin() const {   
        return const_iterator(*this, 0);
    }
    const_iterator end() const {
        return const_iterator(*this, data.capacity);
    }
    iterator begin() {   
        return iterator(*this, 0);
    }
    iterator end() {
        return iterator(*this, data.capacity);
    }
};

template<typename K, typename V> 
struct Type_Info<map_slot<K,V>> {
	static constexpr char name[] = "map_slot";
	static constexpr usize size = sizeof(map_slot<K,V>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _key[] = "key";
    static constexpr char _value[] = "value";
    static constexpr char _bucket[] = "_bucket";
	using members = Type_List<Record_Field<K,offset_of(&map_slot<K,V>::key),_key>,
                              Record_Field<V,offset_of(&map_slot<K,V>::value),_value>,
                              Record_Field<u32,offset_of(&map_slot<K,V>::_bucket),_bucket>>;
};

template<typename K, typename V, Hash<K> H, typename A> 
struct Type_Info<map<K,V,H,A>> {
	static constexpr char name[] = "map";
	static constexpr usize size = sizeof(map<K,V,H,A>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _data[] = "data";
    static constexpr char _size[] = "size";
    static constexpr char _probe[] = "probe";
    static constexpr char _usable[] = "usable";
	using members = Type_List<Record_Field<vec<map_slot<K,V>, A>,offset_of(&map<K,V,H,A>::data),_data>,
                              Record_Field<u32,offset_of(&map<K,V,H,A>::size),_size>,
                              Record_Field<u32,offset_of(&map<K,V,H,A>::probe),_probe>,
                              Record_Field<u32,offset_of(&map<K,V,H,A>::usable),_usable>>;
};
