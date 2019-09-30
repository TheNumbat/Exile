
#pragma once

template<typename T, typename A = Mdefault>
struct vec {
	T* data 	 = null;
	u32 size 	 = 0;
	u32 capacity = 0;

	static vec<T> make(u32 capacity = 8) {
		return {A::template make<T>(capacity), 0, capacity};
	}
	static vec<T> copy(vec<T> source) {
		vec<T> ret = {A::template make<T>(source.capacity), source.size, source.capacity};
		memcpy(ret.data, source.data, sizeof(T) * source.size);
		return ret;
	}
	static vec<T> take(vec<T>& source) {
		vec<T> ret = source;
		source = {null, 0, 0};
		return ret;
	}

	void destroy() {
		A::dealloc(data);
		data = null;
		size = capacity = 0;
	}
	
	void grow() {
		u32 new_capacity = capacity ? 2 * capacity : 8;
		
		T* new_data = A::template make<T>(new_capacity);
		memcpy(new_data, data, sizeof(T) * capacity);
		A::dealloc(data);
		
		capacity = new_capacity;
		data = new_data;
	}
	void clear() {
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

	T* begin() const {
		return data;
	}
	T* end() const {
		return data + size;
	}

	struct norefl split {
		const vec<T> l, r;
	};

	split halves() const {
		assert(size > 1);
		i32 r_s = size / 2;
		i32 l_s = r_s + size % 2;
		return {{data, l_s, l_s}, {data + l_s, r_s, r_s}};
	}
};
