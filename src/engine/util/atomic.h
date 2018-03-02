
#pragma once

template<typename E>
struct atomic_enum {

	void set(E val);
	E get();

private:
	u64 value = (u64)E::none;
	friend void make_meta_info();
};
