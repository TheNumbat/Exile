
#pragma once
#include "reflect.h"

template<typename T> 
any any::make(T* val) {
	any ret;
	ret.value = val;
	ret.id = (type_id)typeid(T).hash_code();
	return ret;
}

template<typename E>
string enum_to_string(E val) { 

	return enum_to_string((i64)val, TYPEINFO(E));
}

template<typename E> 
E string_to_enum(string name) {

	_type_info* info = TYPEINFO(E);
	for(u32 i = 0; i < info->_enum.member_count; i++) {
		if(name == info->_enum.member_names[i]) {
			return (E)info->_enum.member_values[i];
		}
	}
	return E::none;
}
