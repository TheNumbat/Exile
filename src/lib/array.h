
#pragma once

template<typename T, u32 N>
struct array {
	T data[N] 	 = {};
	static constexpr u32 capacity = N;

	static array<T,N> copy(array<T,N> source) {
		array<T,N> ret;
		memcpy(ret.data, source.data, sizeof(T) * N);
		return ret;
	}
	
	T& operator[](u32 idx) {
		assert(idx >= 0 && idx < N);
		return data[idx];
	}
	T operator[](u32 idx) const {
		assert(idx >= 0 && idx < N);
		return data[idx];
	}

	const T* begin() const {
		return data;
	}
	const T* end() const {
		return data + N;
	}
	T* begin() {
		return data;
	}
	T* end() {
		return data + N;
	}
};
