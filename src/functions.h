
#pragma once
#ifndef __clang__

template<typename T>
struct return_type {};

template<typename T, typename... args>
struct func_ptr {
	func_ptr();
	~func_ptr();

	T (*func)(args...) = null;
	string name;

	T operator()(args...) {

		if(global_func->reloading) {
			reload();
			global_func->ptrs_reloaded++;
			if(global_func->ptrs_reloaded == global_func->ptrs_existing) {
				global_func->reloading = false;
			}
		}

		return func(args...);
	}

	void set(T (*f)(args...), string n);

	void reload();
};

struct func_ptr_state {
	u32 ptrs_existing = 0;
	u32 ptrs_reloaded = 0;
	bool reloading = true;
	platform_dll* this_dll = null;
	
	func_ptr<void> test;
};

static func_ptr_state* global_func = null;

#define FPTR(name) name, string_literal(#name)

void test_funcs();

#endif
