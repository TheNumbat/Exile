
#pragma once

struct _FPTR {
	void* func;
	string name;
};

#define MAX_FPTRS 128

struct func_ptr_state {

	_FPTR all_ptrs[128];
	u32 num_ptrs = 0;

	platform_dll* this_dll = null;
	platform_mutex mut;

	void reload_all();
};

static func_ptr_state* global_func = null;

#ifndef __clang__
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
#endif

_FPTR* _fptr(void* func, string name);

void setup_fptrs();
void cleanup_fptrs();

#define FPTR(name) _fptr(&name, string::literal(#name))

