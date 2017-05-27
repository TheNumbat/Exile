
#pragma once

template<typename T>
struct stack {vector<T> contents;};

template<typename T> stack<T> make_stack(u32 capacity, allocator* a);
template<typename T> stack<T> make_stack(u32 capacity = 0);
template<typename T> stack<T> make_stack_copy(stack<T> src);
template<typename T> void destroy_stack(stack<T>* s);

template<typename T> void stack_push(stack<T>* s, T value);
template<typename T> T stack_pop(stack<T>* s);

template<typename T> T stack_top(stack<T>* s);
template<typename T> bool stack_empty(stack<T>* s);
