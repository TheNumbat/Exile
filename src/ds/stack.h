
#pragma once

template<typename T>
struct stack {
	vector<T> contents;
};

template<typename T> stack<T> make_stack(u32 capacity, allocator* a);
template<typename T> stack<T> make_stack(u32 capacity = 0);
template<typename T> stack<T> make_stack_copy(stack<T> src);
template<typename T> stack<T> make_stack_copy(stack<T> src, allocator* a);
template<typename T> stack<T> make_stack_copy_trim(stack<T> src, allocator* a);
template<typename T> void destroy_stack(stack<T>* s);
template<typename T> void clear_stack(stack<T>* s);

template<typename T> void stack_push(stack<T>* s, T value);
template<typename T> T stack_pop(stack<T>* s);
template<typename T> bool stack_try_pop(stack<T>* s, T* out);

template<typename T> T* stack_top(stack<T>* s);
template<typename T> bool stack_empty(stack<T>* s);

template<typename T>
struct con_stack {
	vector<T> contents;
	platform_mutex mut;
	platform_semaphore sem;
};

template<typename T> con_stack<T> make_con_stack(u32 capacity, allocator* a);
template<typename T> con_stack<T> make_con_stack(u32 capacity = 0);
template<typename T> void destroy_con_stack(con_stack<T>* s);

template<typename T> T* stack_push(con_stack<T>* s, T value);
template<typename T> T stack_wait_pop(con_stack<T>* s);
template<typename T> bool stack_try_pop(con_stack<T>* s, T* out);
