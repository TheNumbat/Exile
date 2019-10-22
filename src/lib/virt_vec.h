
#pragma once

template<typename T, usize N = MB(128)>
struct v_vec {
    T* data 	 = null;
    u32 size 	 = 0;

    // note that the new virtual memory is zero-initialized, not constructed!!
    static v_vec make() {
        return {Mvirtual::template alloc<T>(N), 0};
    }
    template<usize SN>
    static v_vec copy(v_vec<T,SN> source) {
        static_assert(N == SN, "Trying to copy to a different sized v_vec!");
        v_vec ret = {Mvirtual::template alloc<T>(source.capacity), source.size};
        memcpy(ret.data, source.data, sizeof(T) * source.size);
        return ret;
    }
    static v_vec take(v_vec& source) {
        v_vec ret = source;
        source = {null, 0};
        return ret;
    }

    void destroy() {
        if constexpr(is_Destroy<T>()) {
            for(T& v : *this) {
                v.destroy();
            }
        }
        Mvirtual::dealloc(data);
        data = null;
        size = 0;
    }
    
    void clear() {
        destroy();
        *this = make();
    }
    bool empty() const {
        return size == 0;
    }

    T& push(T value) {
        if(!data) *this = make();
        assert(size < N);
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

template<typename T, usize N> 
struct Type_Info<v_vec<T,N>> {
	static constexpr char name[] = "v_vec";
	static constexpr usize size = sizeof(v_vec<T,N>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _data[] = "data";
    static constexpr char _size[] = "size";
	using members = Type_List<Record_Field<T*,offset_of(&v_vec<T,N>::data),_data>,
                              Record_Field<u32,offset_of(&v_vec<T,N>::size),_size>>;
};