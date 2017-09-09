
#pragma once
#ifndef __clang__

struct func_ptr_state;
static func_ptr_state* global_func = null;

struct _FPTR {
	void* func;
	string name;
};

template<typename T, typename... args>
struct func_ptr {

	_FPTR* data = null;

	T operator()(args... arg) {

		return ((T(*)(args...))data->func)(arg...);
	}

	void set(_FPTR* f) {
		data = f;
	}
};

_FPTR* _fptr(void* func, string name);

#define FPTR(name) _fptr(&name, string_literal(#name))

#endif
