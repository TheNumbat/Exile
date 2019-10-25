
#pragma once

template<typename T, typename A = Mdefault>
struct queue {
    T* data = null;
    // end points past the last element. size is the number of elements preceeding it (may wrap around)
    u32 size = 0, last = 0, capacity = 0;

    // note that a just-declared (i.e. zero'd) queue is already a valid queue
    static queue make(u32 capacity) {
        return {A::template make<T>(capacity), 0, 0, capacity};
    }
    static queue copy(queue source) {
        queue ret = {A::template make<T>(source.capacity), source.start, source.last, source.capacity};
        memcpy(ret.data, source.data, sizeof(T) * source.capacity);
        return ret;
    }
    static queue take(queue& source) {
        queue ret = source;
        source = {null, 0, 0, 0};
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
        size = last = capacity = 0;
    }

    void grow() {
        u32 new_capacity = capacity ? 2 * capacity : 8;

        T* new_data = A::template make<T>(new_capacity);
        
        T* start = data + last - size;

        if(size <= last) memcpy(new_data, start, sizeof(T) * size);
        else {
            u32 first = size - last;
            memcpy(new_data, start + capacity, sizeof(T) * first);
            memcpy(new_data + first, data, sizeof(T) * last);
        }

        A::dealloc(data);

        last = size;
        capacity = new_capacity;
        data = new_data;
    }

    T* push(T value) {
        if(size == capacity) grow();

        data[last] = value;
        T* ret = &data[last];

        size++;
        last = last == capacity - 1 ? 0 : last + 1;
        return ret;
    }
    T pop() {
        assert(size > 0);
        u32 idx = size <= last ? last - size : last - size + capacity;
        size--;
        return data[idx];
    }
    void clear() {
        if constexpr(is_Destroy<T>()) {
            for(T& v : *this) {
                v.destroy();
            }
        }
        size = last = 0;
    }

    bool empty() {
        return size == 0;
    }
    bool full() {
        return size == capacity;
    }

    template<typename E>
    struct itr {
        itr(queue& _q, u32 idx, u32 cons) : q(_q), place(idx), consumed(cons) {}

        itr operator++() { place = place == q.capacity - 1 ? 0 : place + 1; consumed++; return *this; }
        itr operator++(int) { itr i = *this; place = place == q.capacity - 1 ? 0 : place + 1; consumed++; return i; }
        E& operator*() { return q.data[place]; }
        E* operator->() { return &q.data[place]; }
        bool operator==(const itr& rhs) { return &q == &rhs.q && place == rhs.place && consumed == rhs.consumed; }
        bool operator!=(const itr& rhs) { return &q != &rhs.q || place != rhs.place || consumed != rhs.consumed; }
        
        queue& q;
        u32 place, consumed;
    };
    typedef itr<T> iterator;
    typedef itr<const T> const_iterator;

    u32 start_idx() const {
        return size <= last ? last - size : last - size + capacity;
    }
    const_iterator begin() const {   
        return const_iterator(*this, start_idx(), 0);
    }
    const_iterator end() const {
        return const_iterator(*this, last, size);
    }
    iterator begin() {   
        return iterator(*this, start_idx(), 0);
    }
    iterator end() {
        return iterator(*this, last, size);
    }
};

template<typename T, typename A> 
struct Type_Info<queue<T,A>> {
	static constexpr char name[] = "queue";
	static constexpr usize size = sizeof(queue<T,A>);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _data[] = "data";
    static constexpr char _size[] = "size";
    static constexpr char _last[] = "last";
    static constexpr char _capacity[] = "capacity";
	using members = Type_List<Record_Field<T*,offset_of(&queue<T,A>::data),_data>,
                              Record_Field<u32,offset_of(&queue<T,A>::size),_size>,
                              Record_Field<u32,offset_of(&queue<T,A>::last),_last>,
                              Record_Field<u32,offset_of(&queue<T,A>::capacity),_capacity>>;
};
