
#pragma once

// don't take addresses of elements in a vector, they will be invalidated if the vector grows

// Vector foreach
#define FORVEC(it, v,code)		{u32 __##it = 0; for(auto it = (v).memory; it != (v).memory + (v).size; __##it++, it++) {code}}
#define FORVECCAP(it,v,code) 	{u32 __##it = 0; for(auto it = (v).memory; it != (v).memory + (v).capacity; __##it++, it++) {code}}
#define FORVEC_R(it,v,code)		{u32 __##it = (v).size; for(auto it = (v).memory + (v).size; it != (v).memory; __##it--, it--) {code}}

template<typename T>
struct vector {
	T* memory 	 	 = null;
	u32 size 	 	 = 0;
	u32 capacity 	 = 0;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static vector<T> make(u32 capacity, allocator* a);
	static vector<T> make(u32 capacity = 8);
	static vector<T> make_copy(vector<T> source);
	static vector<T> make_copy(vector<T> source, allocator* a);
	static vector<T> make_copy_trim(vector<T> source, allocator* a);
	void destroy();
	void clear(); // doesn't free/resize

	void grow();
	void zero();
	void resize(u32 capacity); // a smaller size will truncate
	T* push(T value);
	void pop();
	void pop_front();
	void erase(u32 index, u32 num = 1);
	void erase(T val); // linear search, removes all found

	T* get(u32 idx);
	T* front();
	T* back();
	T* find(T val); // linear search

	void qsort(u32 low = 0, u32 high = 0, bool first = true); // quick sort
	u32 partition(u32 low, u32 high);

	bool empty();
};
