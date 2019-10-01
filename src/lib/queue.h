
#pragma once

template<typename T, typename A = Mdefault>
struct queue {
	T* data = null;
    // end points past the last element. size is the number of elements preceeding it (may wrap around)
    u32 size = 0, last = 0, capacity = 0;

    // note that a just-declared (i.e. zero'd) queue is already a valid queue
	static queue<T> make(u32 capacity) {
        return {A::template make<T>(capacity), 0, 0, capacity};
	}
	static queue<T> copy(queue<T> source) {
        queue<T> ret = {A::template make<T>(source.capacity), source.start, source.last, source.capacity};
        memcpy(ret.data, source.data, sizeof(T) * capacity);
        return ret;
	}
	static queue<T> take(queue<T>& source) {
        queue<T> ret = source;
        source = {null, 0, 0, 0};
        return ret;
	}

	void destroy() {
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

        last = size;
        capacity = new_capacity;
        data = new_data;
    }

	void push(T value) {
        if(size == capacity) grow();

        data[last] = value;
        size++;
        last = last == capacity - 1 ? 0 : last + 1;
	}
	T pop() {
        assert(size > 0);
        size--;
        last = last == 0 ? capacity - 1 : last - 1;
        return data[last];
	}
    void clear() {
        size = last = 0;
    }

	T* begin() const {
        
	}
	T* end() const {
	}

    template<typename E>
    struct itr {
        // typedef iterator self_type;
        // typedef T value_type;
        // typedef T& reference;
        // typedef T* pointer;
        // typedef std::forward_iterator_tag iterator_category;
        // typedef int difference_type;
        
        iterator(queue<T,A>& _q) : q(_q), place(0) {}

        itr operator++() { itr i = *this; place = place == q.capacity - 1 ? 0 : place + 1; return i; }
        itr operator++(int) { place = place == q.capacity - 1 ? 0 : place + 1; return *this; }
        E& operator*() { return q.data[place]; }
        E* operator->() { return &q.data[place]; }
        bool operator==(const itr& rhs) { return &q == &rhs.q && place == rhs.place; }
        bool operator!=(const itr& rhs) { return &q != &rhs.q || place != rhs.place; }
        
        queue<T,A>& q;
        u32 place;
    };
    typedef itr<T> iterator;
    typedef itr<const T> const_iterator;
};
