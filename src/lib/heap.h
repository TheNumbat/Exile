
#pragma once

// note: the heap iterator iterates the array linearly, not in heap order

template<typename T, typename A = Mdefault>
struct heap {
    T* data 	 = null;
    u32 size 	 = 0;
    u32 capacity = 0;

    static heap make(u32 capacity) {
        return {A::template make<T>(capacity), 0, capacity};
    }
    static heap copy(heap source) {
        heap ret = {A::template make<T>(source.capacity), source.size, source.capacity};
        memcpy(ret.data, source.data, sizeof(T) * source.size);
        return ret;
    }
    static heap take(heap& source) {
        heap ret = source;
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

    void push(T value) {
        if(full()) grow();
        data[size++] = value;
        reheap_up(size - 1);
    }
    T pop() {
        assert(size > 0);
        T ret = data[0];
        if(--size) {
            data[0] = data[size];
            reheap_down(0);
        }
        return ret;
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

private:
    void reheap_up(u32 idx) {
        if(!idx) return;
        u32 parent = (idx - 1) / 2;
        
        T tmp = data[idx];
        T par = data[parent];
        if(tmp > par) {
            data[idx] = par;
            data[parent] = tmp;
            reheap_up(parent);
        }
    }

    void reheap_down(u32 idx) {
        T val = data[idx];
        
        u32 left = idx * 2 + 1;
        u32 right = left + 1;

        if(right < size) {
            T lval = data[left];
            T rval = data[right];
            if(lval > val && lval >= rval) {
                data[idx] = lval;
                data[left] = val;
                reheap_down(left);
            } else if(rval > val && rval >= lval) {
                data[idx] = rval;
                data[right] = val;
                reheap_down(right);
            }
        } else if(left < size) {
            T lval = data[left];
            if(lval > val) {
                data[idx] = lval;
                data[left] = val;
            }
        }
    }
};

template<typename T, typename A> 
struct Type_Info<heap<T,A>> {
	static constexpr char name[] = "heap";
	static constexpr usize size = sizeof(heap<T,A>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _data[] = "data";
    static constexpr char _size[] = "size";
    static constexpr char _capacity[] = "capacity";
    typedef heap<T,A> __offset;
	using members = Type_List<Record_Field<T*,offsetof(__offset, data),_data>,
                              Record_Field<u32,offsetof(__offset, size),_size>,
                              Record_Field<u32,offsetof(__offset, capacity),_capacity>>;
};
