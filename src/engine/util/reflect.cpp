
#include "reflect.h"
#include "../log.h"
#include "../dbg.h"

thread_local map<type_id,_type_info> type_table;

_type_info* any::info() {
	return TYPEINFO_H(id);
}
	
i64 int_as_i64(void* val, _type_info* info) {

	switch(info->size) {
	case 1: {
		if(info->_int.is_signed) {
			return (i64)*(i8*)val;
		} else {
			return (i64)*(u8*)val;
		}
	} break;
	case 2: {
		if(info->_int.is_signed) {
			return (i64)*(i16*)val;
		} else {
			return (i64)*(u16*)val;
		}
	} break;
	case 4: {
		if(info->_int.is_signed) {
			return (i64)*(i32*)val;
		} else {
			return (i64)*(u32*)val;
		}
	} break;
	case 8: {
		if(info->_int.is_signed) {
			return (i64)*(i64*)val;
		} else {
			return (i64)*(u64*)val;
		}
	} break;
	default: {
		INVALID_PATH;
		return 0;
	} break;
	}
}

void int_from_i64(i64 val, void* i, _type_info* info) {

	switch(info->size) {
	case 1: {
		if(info->_int.is_signed) {
			*(i8*)i = (i8)val;
		} else {
			*(u8*)i = (u8)val;
		}
	} break;
	case 2: {
		if(info->_int.is_signed) {
			*(i16*)i = (i16)val;
		} else {
			*(u16*)i = (u16)val;
		}
	} break;
	case 4: {
		if(info->_int.is_signed) {
			*(i32*)i = (i32)val;
		} else {
			*(u32*)i = (u32)val;
		}
	} break;
	case 8: {
		if(info->_int.is_signed) {
			*(i64*)i = (i64)val;
		} else {
			*(u64*)i = (u64)val;
		}
	} break;
	default: {
		INVALID_PATH;
	} break;
	}	
}
	
f64 float_as_f64(void* val, _type_info* info) {

	switch(info->size) {
	case 4: {
		return (f64)*(f32*)val;
	} break;
	case 8: {
		return *(f64*)val;
	} break;
	default: {
		INVALID_PATH;
		return 0;
	} break;
	}
}

string enum_to_string(i64 val, _type_info* info) { 

	if (!info) return "UNDEF"_;

	string name;

	u32 low = 0, high = info->_enum.member_count;
	for(;;) {

		u32 search = low + ((high - low) / 2);

		i64 mem = info->_enum.member_values[search];

		if(val == mem) {
			name = info->_enum.member_names[search];
			break;
		}

		if(mem < val) {
			low = search + 1;
		} else {
			high = search;
		}

		if(low == high) {
			break;
		}
	}

	return name;
}

void make_type_table(allocator* alloc) { 

	type_table = map<type_id,_type_info>::make(1024, alloc);

	make_meta_info();
}
