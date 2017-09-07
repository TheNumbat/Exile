
#pragma once
#ifndef __clang__

struct func_ptr_state;
static func_ptr_state* global_func = null;

struct _FPTR {
	void* data;
	string n;
};

template<typename T, typename... args>
struct func_ptr {

	T (*func)(args...) = null;
	string name;

	u32 id = 0;

	T operator()(args... arg) {

		return func(arg...);
	}

	void set(_FPTR f) {
		func = (T(*)(args...))f.data;
		name = f.n;

		if(id == 0) {
			id = global_func->next_id++;
			global_func->all_ptrs.insert(id, (_FPTR*)this);
		}
	}
};

#define FPTR(name) {&name, string_literal(#name)}

#endif
