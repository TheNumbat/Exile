
#pragma once
#ifndef __clang__

struct _FPTR {
	void* data;
	string n;
};

template<typename T, typename... args>
struct func_ptr {

	T (*func)(args...) = null;
	string name;

	T operator()(args... arg) {

		return func(arg...);
	}

	void set(_FPTR f) {
		func = (T(*)(args...))f.data;
		name = f.n;

		global_func->all_func_ptrs[global_func->current_func_ptrs++] = (_FPTR*)this;
	}
};

#define MAX_FUNC_PTRS 256
struct func_ptr_state {

	_FPTR* all_func_ptrs[MAX_FUNC_PTRS] = {};
	u32 current_func_ptrs = 0;

	platform_dll* this_dll = null;

	void reload_all();
};

static func_ptr_state* global_func = null;

#define FPTR(name) {&name, string_literal(#name)}

#endif
