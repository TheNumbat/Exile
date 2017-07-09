
#pragma once

// don't take addresses of elements in a vector, they will be invalidated if the vector grows

template<typename T>
struct vector { // @VEC
	u32 size 	 	 = 0;
	u32 capacity 	 = 0;
	T* memory 	 	 = NULL;
	allocator* alloc = NULL;
};

template<typename T> vector<T> make_vector(u32 capacity, allocator* a);
template<typename T> vector<T> make_vector(u32 capacity = 4);
template<typename T> vector<T> make_vector_copy(vector<T> source);
template<typename T> vector<T> make_vector_copy(vector<T> source, allocator* a);
template<typename T> vector<T> make_vector_copy_trim(vector<T> source, allocator* a);
template<typename T> void destroy_vector(vector<T>* v);
template<typename T> void clear_vector(vector<T>* v); // doesn't free/resize

template<typename T> void vector_grow(vector<T>* v, bool copy = true);
template<typename T> void vector_resize(vector<T>* v, u32 capacity, bool copy = true); // a smaller size will truncate
template<typename T> void vector_push(vector<T>* v, T value);
template<typename T> void vector_pop(vector<T>* v);
template<typename T> void vector_pop_front(vector<T>* v);
template<typename T> void vector_erase(vector<T>* v, u32 index, u32 num = 1);
template<typename T> void vector_erase(vector<T>* v, T val);

template<typename T> T* vector_get(vector<T>* v, u32 idx);
template<typename T> T* vector_front(vector<T>* v);
template<typename T> T* vector_back(vector<T>* v);
template<typename T> T* vector_find(vector<T>* v, T val); // linear search

template<typename T> void vector_qsort(vector<T>* v, u32 low = 0, u32 high = 0, bool first = true); // quick sort
template<typename T> u32 vector_partition(vector<T>* v, u32 low, u32 high);

template<typename T> u32 vector_len(vector<T>* v);
template<typename T> u32 vector_capacity(vector<T>* v);
template<typename T> bool vector_empty(vector<T>* v);

