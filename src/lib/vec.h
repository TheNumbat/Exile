
#pragma once

template<typename T, typename A = Mdefault>
struct vec {
    T* data 	 = null;
    u32 size 	 = 0;
    u32 capacity = 0;

    // note that a just-declared (i.e. zero'd) vec is already a valid vec
    static vec make(u32 capacity) {
        return {A::template make<T>(capacity), 0, capacity};
    }
    static vec copy(vec source) {
        vec ret = {A::template make<T>(source.capacity), source.size, source.capacity};
        memcpy(ret.data, source.data, sizeof(T) * source.size);
        return ret;
    }
    static vec take(vec& source) {
        vec ret = source;
        source = {null, 0, 0};
        return ret;
    }

    void destroy() {
        if constexpr(is_Destroy<T>()) {
            for(T& v : *this) {
                v.destroy();
            }
        }
        A::dealloc(data);
        data = null;
        size = capacity = 0;
    }
    
    void grow() {
        u32 new_capacity = capacity ? 2 * capacity : 8;
        
        T* new_data = A::template make<T>(new_capacity);
        memcpy(new_data, data, sizeof(T) * size);
        A::dealloc(data);
        
        capacity = new_capacity;
        data = new_data;
    }
    void clear() {
        if constexpr(is_Destroy<T>()) {
            for(T& v : *this) {
                v.destroy();
            }
        }
        size = 0;
    }

    bool empty() const {
        return size == 0;
    }
    bool full() const {
        return size == capacity;
    }

    T& push(T value) {
        if(full()) grow();
        assert(size < capacity);
        data[size] = value;
        return data[size++];
    }
    T pop() {
        assert(size > 0);
        return data[size--];
    }

    T& operator[](u32 idx) {
        assert(idx >= 0 && idx < size);
        return data[idx];
    }
    T operator[](u32 idx) const {
        assert(idx >= 0 && idx < size);
        return data[idx];
    }

    const T* begin() const {
        return data;
    }
    const T* end() const {
        return data + size;
    }
    T* begin() {
        return data;
    }
    T* end() {
        return data + size;
    }

    struct split {
    	const vec l, r;
    };

    split halves() const {
    	assert(size > 1);
    	i32 r_s = size >> 1;
    	i32 l_s = r_s + size & 1;
    	return {{data, l_s, l_s}, {data + l_s, r_s, r_s}};
    }
};

// TODO(max): is this really a useful abstraction? We need something like this in order
// to make use of arenas without conflating allocating vecs with vecs on arena memory.

// NOTE(max): Basically a vec that does not own its memory, i.e. can edit the contained
// data but cannot change the actual allocated memory.
template<typename T>
struct vec_view {
    T* data = null;
    u32 size = 0;
    u32 capacity = 0;

    template<typename A>
    static vec_view from(vec<T,A> v) {
        return {v.data, v.size, v.capacity};
    }
    static vec_view make(T* data, u32 cap, u32 size = 0) {
        return {data, size, cap};
    }

    void clear() {
        if constexpr(is_Destroy<T>()) {
            for(T& v : *this) {
                v.destroy();
            }
        }
        size = 0;
    }

    bool empty() const {
        return size == 0;
    }
    bool full() const {
        return size == capacity;
    }

    T& push(T value) {
        assert(size < capacity);
        data[size] = value;
        return data[size++];
    }
    T pop() {
        assert(size > 0);
        return data[size--];
    }

    T& operator[](u32 idx) {
        assert(idx >= 0 && idx < size);
        return data[idx];
    }
    T operator[](u32 idx) const {
        assert(idx >= 0 && idx < size);
        return data[idx];
    }

    const T* begin() const {
        return data;
    }
    const T* end() const {
        return data + size;
    }
    T* begin() {
        return data;
    }
    T* end() {
        return data + size;
    }
};

template<typename T, typename A> 
struct Type_Info<vec<T,A>> {
	static constexpr char name[] = "vec";
	static constexpr usize size = sizeof(vec<T,A>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _data[] = "data";
    static constexpr char _size[] = "size";
    static constexpr char _capacity[] = "capacity";
	using members = Type_List<Record_Field<T*,offset_of(&vec<T,A>::data),_data>,
                              Record_Field<u32,offset_of(&vec<T,A>::size),_size>,
                              Record_Field<u32,offset_of(&vec<T,A>::capacity),_capacity>>;
};

template<typename T> 
struct Type_Info<vec_view<T>> {
	static constexpr char name[] = "vec_view";
	static constexpr usize size = sizeof(vec_view<T>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _data[] = "data";
    static constexpr char _size[] = "size";
    static constexpr char _capacity[] = "capacity";
	using members = Type_List<Record_Field<T*,offset_of(&vec_view<T>::data),_data>,
                              Record_Field<u32,offset_of(&vec_view<T>::size),_size>,
                              Record_Field<u32,offset_of(&vec_view<T>::capacity),_capacity>>;
};
