
#pragma once
#include "string.h"
#include "../util/reflect.h"

template<typename... Targs>
void string::writef(string fmt, Targs... args) { 
	u32 idx = _string_printf(*this, 0, fmt, false, args...);
	c_str[idx++] = 0;
}

template<typename... Targs>
u32 size_stringf(string fmt, Targs... args) { 
	string tmp;
	return _string_printf(tmp, 0, fmt, true, args...) + 1;
}

template<typename... Targs> 
string string::makef(u32 len, string fmt, Targs... args) { 

	string ret;
	ret 	= string::make(len);
	ret.len = len;
	ret.c_str[len - 1] = 0;

	u32 used = _string_printf(ret, 0, fmt, false, args...);

	LOG_DEBUG_ASSERT(used == len - 1);
	(void)used;

	return ret;
}

template<typename... Targs>
string string::makef(string fmt, Targs... args) { 

	string ret;
	u32 len = size_stringf(fmt, args...);
	ret 	= string::make(len);
	ret.len = len;
	ret.c_str[len - 1] = 0;

	u32 used = _string_printf(ret, 0, fmt, false, args...);

	LOG_DEBUG_ASSERT(used == len - 1);
	(void)used;

	return ret;
}

template<typename... Targs>
string string::makef(string fmt, allocator* a, Targs... args) { 

	string ret;
	u32 len = size_stringf(fmt, args...);
	ret 	= string::make(len, a);
	ret.len = len;
	ret.c_str[len - 1] = 0;

	u32 used = _string_printf(ret, 0, fmt, false, args...);

	LOG_DEBUG_ASSERT(used == len - 1);
	(void)used;

	return ret;
}

template<typename T, typename... Targs> 
T& get_pack_first(T& val, Targs... args) { 
	return val;
}

template<typename T, typename... Targs>
u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size, T val, Targs... args) { 
	return _string_printf(out, idx, fmt, size, args...);
}

template<typename T, typename... Targs>
u32 _string_printf(string out, u32 idx, string fmt, bool size, T& value, Targs... args) { 

	// TODO(max): this function is a garbage fire

	for(u32 i = 0; i < fmt.len - 1; i++) {
		if(fmt.c_str[i] == '%') {
			if(fmt.c_str[i + 1] == '%') {
				idx = out.write(idx, '%', size);
				i++;
			} else {
				if(fmt.c_str[i + 1] == '-') { // left justify
					bool len_in_param = false;
					
					i++;
					if(fmt.c_str[i + 1] == '-') { // cut off excess chars
						i++;
						
						u32 pad = 0;
						u32 consumed = 1;
						string place;

						if(fmt.c_str[i + 1] == '*') {	// total length passed as a parameter
							pad = *(u32*)&value;			
							i++;
							len_in_param = true;
							place = string::makef("%"_, get_pack_first(args...));
						} else {	// total length passed in format string
							i++;
							pad = fmt.parse_u32(i, &consumed);
							i += consumed;
							place = string::makef("%"_, value);
						}

						if(place.len > pad) {
							idx = out.write(idx, place.substring(0, pad - 1), size);
						} else {
							idx = out.write(idx, place, size);
							u32 left = pad - place.len;
							while(left--) {
								idx = out.write(idx, ' ', size);
							}
						}
						place.destroy();
						if (len_in_param) {
							return _string_printf_fwd(out, idx, string::from_c_str(fmt.c_str + i + 1), size, args...);
						} else {
							return _string_printf(out, idx, string::from_c_str(fmt.c_str + i), size, args...);
						}
					}

					i32 pad = 0;
					i32 sized = 0;

					if(fmt.c_str[i + 1] == '*') {	// total length passed as a parameter
						pad = *(u32*)&value;			
						i++;
						len_in_param = true;

						auto param = get_pack_first(args...);
						u32 tmp_idx = out.write_type(idx, &param, TYPEINFO(decltype(param)), size);
						sized = tmp_idx - idx;
						idx = tmp_idx;
					} else {	// total length passed in format string
						u32 len = 0;
						i++;
						pad = fmt.parse_u32(i, &len);
						i += len;
						u32 tmp_idx = out.write_type(idx, &value, TYPEINFO(T), size);
						sized = tmp_idx - idx;
						idx = tmp_idx;
					}

					while(sized < pad) {
						idx = out.write(idx, ' ', size);
						sized++;
					}
					
					if (len_in_param) {
						return _string_printf_fwd(out, idx, string::from_c_str(fmt.c_str + i + 1), size, args...);
					} else {
						return _string_printf(out, idx, string::from_c_str(fmt.c_str + i), size, args...);
					}
				} if(fmt.c_str[i + 1] == '+') { // right justify
					bool len_in_param = false;
					i++;

					u32 pad = 0;
					u32 sized = 0;

					if(fmt.c_str[i + 1] == '*') {	// total length passed as a parameter
						pad = *(u32*)&value;			
						i++;
						len_in_param = true;

						auto param = get_pack_first(args...);
						sized = out.write_type(idx, &param, TYPEINFO(decltype(param)), true) - idx;
					} else {	// total length passed in format string
						u32 len = 0;
						i++;
						pad = fmt.parse_u32(i, &len);
						i += len;
						sized = out.write_type(idx, &value, TYPEINFO(T), true) - idx;
					}

					while(sized < pad) {
						idx = out.write(idx, " "_, size);
						sized++;
					}
					
					if (len_in_param) {
						auto param = get_pack_first(args...);
						idx = out.write_type(idx, &param, TYPEINFO(decltype(param)), size);
						return _string_printf_fwd(out, idx, string::from_c_str(fmt.c_str + i + 1), size, args...);;
					} else {
						idx = out.write_type(idx, &value, TYPEINFO(T), size);
						return _string_printf(out, idx, string::from_c_str(fmt.c_str + i), size, args...);
					}
				} else {
					idx = out.write_type(idx, &value, TYPEINFO(T), size);
					return _string_printf(out, idx, string::from_c_str(fmt.c_str + i + 1), size, args...);
				}
			}
		} else {
			if(!size) {
				out.c_str[idx] = fmt.c_str[i];
			}
			idx++;
		}
	}
	return idx;
}

template<typename... Targs>
string make_stringf_a(allocator* a, string fmt, Targs... args) { 

	string ret;
	u32 len = size_stringf(fmt, args...);
	ret 	= string::make(len, a);
	ret.len = len;
	ret.c_str[len - 1] = 0;

	_string_printf(ret, 0, fmt, false, args...);

	return ret;
}

