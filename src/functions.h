
#pragma once
#ifndef __clang__

template<typename T>
struct func_ptr {
	func_ptr();
	~func_ptr();

	T* func = null;
	string name;

	template<typename... args>
	auto operator()(args...);

	void set(T* f, string n);

	void reload();
};

struct func_ptr_state {
	u32 ptrs_existing = 0;
	u32 ptrs_reloaded = 0;
	bool reloading = true;
	platform_dll* this_dll = null;
	
	func_ptr<void()> test;
};

static func_ptr_state* global_func = null;

#define FPTR(name) name, string_literal(#name)

void test_funcs();

#endif
