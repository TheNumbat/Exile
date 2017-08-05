
u32 parse_u32(string s, u32 idx, u32* used) { PROF

	u32 accum = 0;
	char* place = s.c_str + idx;
	while(*place >= '0' && *place <= '9') {
		accum *= 10;
		accum += *place - '0';
		place++;
		if(used) (*used)++;
	}

	return accum;
}

u32 string_insert(string s, u32 idx, string ins, bool size) { 

#ifdef MORE_PROF
	PROF
#endif

	if(ins.len == 0) return idx;
	if(!size) memcpy(ins.c_str, s.c_str + idx, ins.len - 1);
	return idx + ins.len - 1;
}

u32 string_insert(string s, u32 idx, char ins, bool size) { 

#ifdef MORE_PROF
	PROF
#endif

	if(!size) s.c_str[idx] = ins;
	return idx + 1;
}

template<typename... Targs>
void string_printf(string out, string fmt, Targs... args) { PROF
	u32 idx = _string_printf(out, 0, fmt, false, args...);
	out.c_str[idx++] = 0;
}

template<typename... Targs>
u32 size_stringf(string fmt, Targs... args) { PROF
	string tmp;
	return _string_printf(tmp, 0, fmt, true, args...) + 1;
}

template<typename... Targs> 
string make_stringf_len(u32 len, string fmt, Targs... args) { PROF

	string ret;
	ret 	= make_string(len);
	ret.len = len;
	ret.c_str[len - 1] = 0;

	_string_printf(ret, 0, fmt, false, args...);

	return ret;
}

template<typename... Targs>
string make_stringf(string fmt, Targs... args) { PROF

	string ret;
	u32 len = size_stringf(fmt, args...);
	ret 	= make_string(len);
	ret.len = len;
	ret.c_str[len - 1] = 0;

	_string_printf(ret, 0, fmt, false, args...);

	return ret;
}

template<typename T, typename... Targs> 
inline T& get_pack_first(T& val, Targs... args) { PROF
	return val;
}
inline u32 get_pack_first() { PROF
	return 0;
}

template<typename T, typename... Targs>
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size, T val, Targs... args) { PROF
	return _string_printf(out, idx, fmt, size, args...);
}
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size) { PROF
	return _string_printf(out, idx, fmt, size);
}

template<typename T, typename... Targs>
u32 _string_printf(string out, u32 idx, string fmt, bool size, T& value, Targs... args) { PROF

	for(u32 i = 0; i < fmt.len - 1; i++) {
		if(fmt.c_str[i] == '%') {
			if(fmt.c_str[i + 1] == '%') {
				idx = string_insert(out, idx, string_literal("%"), size);
				i++;
			} else {
				if(fmt.c_str[i + 1] == '-') { // left justify
					bool len_in_param = false;
					i++;

					u32 pad = 0;
					u32 sized = 0;

					if(fmt.c_str[idx + 1] == '*') {	// total length passed as a parameter
						pad = *(u32*)&value;			
						i++;
						len_in_param = true;

						auto param = get_pack_first(args...);
						u32 tmp_idx = print_type(out, idx, &param, TYPEINFO(decltype(param)), size);
						sized = tmp_idx - idx;
						idx = tmp_idx;
					} else {	// total length passed in format string
						u32 len = 0;
						i++;
						pad = parse_u32(fmt, i, &len);
						i += len;
						u32 tmp_idx = print_type(out, idx, &value, TYPEINFO(T), size);
						sized = tmp_idx - idx;
						idx = tmp_idx;
					}

					while(sized < pad) {
						idx = string_insert(out, idx, string_literal(" "), size);
						sized++;
					}
					
					if (len_in_param) {
						return _string_printf_fwd(out, idx, string_from_c_str(fmt.c_str + i + 1), size, args...);;
					} else {
						return _string_printf(out, idx, string_from_c_str(fmt.c_str + i), size, args...);
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
						sized = print_type(out, idx, &param, TYPEINFO(decltype(param)), true) - idx;
					} else {	// total length passed in format string
						u32 len = 0;
						i++;
						pad = parse_u32(fmt, i, &len);
						i += len;
						sized = print_type(out, idx, &value, TYPEINFO(T), true) - idx;
					}

					while(sized < pad) {
						idx = string_insert(out, idx, string_literal(" "), size);
						sized++;
					}
					
					if (len_in_param) {
						auto param = get_pack_first(args...);
						idx = print_type(out, idx, &param, TYPEINFO(decltype(param)), size);
						return _string_printf_fwd(out, idx, string_from_c_str(fmt.c_str + i + 1), size, args...);;
					} else {
						idx = print_type(out, idx, &value, TYPEINFO(T), size);
						return _string_printf(out, idx, string_from_c_str(fmt.c_str + i), size, args...);
					}
				} else {
					idx = print_type(out, idx, &value, TYPEINFO(T), size);
					return _string_printf(out, idx, string_from_c_str(fmt.c_str + i + 1), size, args...);
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
u32 _string_printf(string out, u32 idx, string fmt, bool size) { PROF
	for(u32 i = 0; i < fmt.len - 1; i++) {
		if(fmt.c_str[i] == '%') {
			if(fmt.c_str[i + 1] == '%') {
				idx = string_insert(out, idx, string_literal("%"), size);
				i++;
			} else {
				LOG_ERR("Missing parameter for string_printf!");
				return idx;
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

u32 print_type(string s, u32 idx, void* val, _type_info* info, bool size) { PROF
	if(info == null) {
		idx = string_insert(s, idx, string_literal("UNDEF"), size);
		return idx;
	}
	switch(info->type_type) {
	case Type::_void: {
		idx = string_insert(s, idx, string_literal("void"), size);
	} break;
	
	case Type::_int: {
		idx = print_int(s, idx, 10, val, info, size); 
	} break;

	case Type::_float: {
		idx = print_float(s, idx, 6, val, info, size);		// need bit size flags
	} break;

	case Type::_bool: {
		idx = string_insert(s, idx, string_literal(*(bool*)val ? "true" : "false"), size);
	} break;

	case Type::_ptr: {
		idx = print_ptr(s, idx, val, info, size);
	} break;

	case Type::_array: {
		idx = print_static_array(s, idx, val, info, size);
	} break;

	case Type::_func: {
		idx = string_insert(s, idx, info->_func.signature, size);
	} break;

	case Type::_struct: {
		if(info->name == "vector") {
			idx = print_vector(s, idx, val, info, size);
		} else if(info->name == "array") {
			idx = print_array(s, idx, val, info, size);
		} else if(info->name == "map") {
			idx = print_map(s, idx, val, info, size);
		} else if(info->name == "queue" || info->name == "con_queue") {
			idx = print_queue(s, idx, val, info, size);
		} else {
			idx = print_struct(s, idx, val, info, size);
		}
	} break;

	case Type::_enum: {
		idx = print_enum(s, idx, val, info, size);
	} break;

	case Type::_string: {
		if (((string*)val)->len) {
			idx = string_insert(s, idx, *(string*)val, size);
		}
	} break;
	}
	return idx;
}

u32 print_array(string s, u32 idx, void* val, _type_info* info, bool size) { PROF

	// TODO(max): assumes binary compatibility with vector T* first member then u32 size
	return print_vector(s, idx, val, info, size);
}

u32 print_queue(string s, u32 idx, void* val, _type_info* info, bool size) { PROF

	idx = string_insert(s, idx, info->name, size);

	_type_info* of = TYPEINFO_H(TYPEINFO_H(info->_struct.member_types[0])->_ptr.to);
	if(!of) {
		idx = string_insert(s, idx, string_literal("[UNDEF]"), size);
		return idx;
	}

	u32 start = *(u32*)((u8*)val + info->_struct.member_offsets[1]);
	u32 end = *(u32*)((u8*)val + info->_struct.member_offsets[2]);
	u32 capacity = *(u32*)((u8*)val + info->_struct.member_offsets[3]);
	u8* data = *(u8**)((u8*)val + info->_struct.member_offsets[0]);

	idx = string_insert(s, idx, '[', size);
	for(u32 i = start; i != end; ++i %= capacity) {
		if(of->type_type == Type::_string) {
			idx = string_insert(s, idx, '\"', size);
		}
		idx = print_type(s, idx, data + i * of->size, of, size);
		if(of->type_type == Type::_string) {
			idx = string_insert(s, idx, '\"', size);
		}
		if(i != end - 1)
			idx = string_insert(s, idx, string_literal(", "), size);
	}
	idx = string_insert(s, idx, ']', size);

	return idx;	
}

u32 print_vector(string s, u32 idx, void* val, _type_info* info, bool size) { PROF

	idx = string_insert(s, idx, info->name, size);

	_type_info* of = TYPEINFO_H(TYPEINFO_H(info->_struct.member_types[0])->_ptr.to);
	if(!of) {
		idx = string_insert(s, idx, string_literal("[UNDEF]"), size);
		return idx;
	}

	// TODO(max): probably shouldn't hard-code locations of members in vector

	u32 len = *(u32*)((u8*)val + info->_struct.member_offsets[1]);
	u8* data = *(u8**)((u8*)val + info->_struct.member_offsets[0]);

	idx = string_insert(s, idx, '[', size);
	for(u32 i = 0; i < len; i++) {
		if(of->type_type == Type::_string) {
			idx = string_insert(s, idx, '\"', size);
		}
		idx = print_type(s, idx, data + i * of->size, of, size);
		if(of->type_type == Type::_string) {
			idx = string_insert(s, idx, '\"', size);
		}
		if(i != len - 1)
			idx = string_insert(s, idx, string_literal(", "), size);
	}
	idx = string_insert(s, idx, ']', size);

	return idx;
}

u32 print_map(string s, u32 idx, void* val, _type_info* info, bool size) { PROF

	_type_info* vec_info = TYPEINFO_H(info->_struct.member_types[0]);
	u8* vec = (u8*)val + info->_struct.member_offsets[0];

	idx = string_insert(s, idx, info->name, size);

	_type_info* vec_of = TYPEINFO_H(TYPEINFO_H(vec_info->_struct.member_types[0])->_ptr.to);
	if(!vec_of) {
		idx = string_insert(s, idx, string_literal("[UNDEF]"), size);
		return idx;
	}

	u32 cap = *(u32*)((u8*)vec + vec_info->_struct.member_offsets[2]);
	u8* data = *(u8**)((u8*)vec + vec_info->_struct.member_offsets[0]);

	u32 number_actually_printed = 0;
	idx = string_insert(s, idx, '[', size);
	for(u32 i = 0; i < cap; i++) {
		if(!*(bool*)(data + i * vec_of->size + vec_of->_struct.member_offsets[2])) continue;

		if(number_actually_printed != 0)
			idx = string_insert(s, idx, string_literal(", "), size);
		number_actually_printed++;
		
		idx = string_insert(s, idx, '{', size);
		{
			_type_info* key = TYPEINFO_H(vec_of->_struct.member_types[0]);
			if(key->type_type == Type::_string) {
				idx = string_insert(s, idx, '\"', size);
			}
			idx = print_type(s, idx, data + i * vec_of->size + vec_of->_struct.member_offsets[0], key, size);
			if(key->type_type == Type::_string) {
				idx = string_insert(s, idx, '\"', size);
			}
		}
		idx = string_insert(s, idx, string_literal(", "), size);
		{
			_type_info* value = TYPEINFO_H(vec_of->_struct.member_types[1]);
			if(value->type_type == Type::_string) {
				idx = string_insert(s, idx, '\"', size);
			}
			idx = print_type(s, idx, data + i * vec_of->size + vec_of->_struct.member_offsets[1], value, size);
			if(value->type_type == Type::_string) {
				idx = string_insert(s, idx, '\"', size);
			}
		}
		idx = string_insert(s, idx, '}', size);
	}
	idx = string_insert(s, idx, ']', size);
	return idx;
}

u32 print_static_array(string s, u32 idx, void* val, _type_info* info, bool size) { PROF 
	
	_type_info* of = TYPEINFO_H(info->_array.of);

	if (!of) {
		idx = string_insert(s, idx, string_literal("UNDEF["), size);
		idx = print_u64(s, idx, 10, info->_array.length, size);
		idx = string_insert(s, idx, ']', size);
		return idx;
	}

	idx = string_insert(s, idx, of->name, size);
	idx = string_insert(s, idx, '[', size);
	for(u32 i = 0; i < info->_array.length; i++) {
		if(of->type_type == Type::_string) {
			idx = string_insert(s, idx, '\"', size);
		}
		idx = print_type(s, idx, (u8*)val + i * of->size, of, size);
		if(of->type_type == Type::_string) {
			idx = string_insert(s, idx, '\"', size);
		}
		if(i != info->_array.length - 1)
			idx = string_insert(s, idx, string_literal(", "), size);
	}

	idx = string_insert(s, idx, ']', size);

	return idx;
}

u32 print_ptr(string s, u32 idx, void* val, _type_info* info, bool size) { PROF 
	idx = string_insert(s, idx, string_literal("*{"), size);
	if (info->_ptr.to == 0) {
		idx = string_insert(s, idx, string_literal("UNDEF|"), size);

		if (*(u8**)&val == null) {
			idx = string_insert(s, idx, string_literal("null"), size);
		} else {
			idx = print_u64(s, idx, 16, (u64)(*(u8**)val), size);
		}
	} else {
		_type_info* to = TYPEINFO_H(info->_ptr.to);
		if (*(u8**)val == null) {
			idx = string_insert(s, idx, to->name, size);
			idx = string_insert(s, idx, string_literal("|null"), size);
		} else {
			if(to->type_type == Type::_string) {
				idx = string_insert(s, idx, '\"', size);
			}			
			idx = print_type(s, idx, *(u8**)val, to, size);
			if(to->type_type == Type::_string) {
				idx = string_insert(s, idx, '\"', size);
			}	
			idx = string_insert(s, idx, '|', size);
			idx = print_u64(s, idx, 16, (u64)(*(u8**)val), size);
		}
	}	
	idx = string_insert(s, idx, '}', size);
	return idx;
}

u32 print_struct(string s, u32 idx, void* val, _type_info* info, bool size) { PROF 
	idx = string_insert(s, idx, info->name, size);
	idx = string_insert(s, idx, string_literal("{"), size);
	for(u32 j = 0; j < info->_struct.member_count; j++) {
		idx = string_insert(s, idx, info->_struct.member_names[j], size);
		idx = string_insert(s, idx, string_literal(" : "), size);

		_type_info* member = TYPEINFO_H(info->_struct.member_types[j]);
		u8* place = (u8*)val + info->_struct.member_offsets[j];

		if(member) {
			if(member->type_type == Type::_string) {
				idx = string_insert(s, idx, '\"', size);
			}
			idx = print_type(s, idx, place, member, size);
			if(member->type_type == Type::_string) {
				idx = string_insert(s, idx, '\"', size);
			}
		} else {
			idx = string_insert(s, idx, string_literal("UNDEF"), size);
		}

		if(j < info->_struct.member_count - 1) {
			idx = string_insert(s, idx, string_literal(", "), size);
		}
	}
	idx = string_insert(s, idx, '}', size);
	return idx;
}

u32 print_u64(string s, u32 idx, u8 base, u64 val, bool size) { PROF

	u8 digit = 0, digits = 0;
	u32 start = idx;
	if(val == 0) {
		idx = string_insert(s, idx, '0', size);
	}
	while(val > 0) {
		digit = val % base;
		val /= base;
		digits++;

		char c = '?';
		if(digit <= 9) {
			c = '0' + digit;
		} else {
			c = 'a' + digit - 9;
		}
		idx = string_insert(s, idx, c, size);
	}

	if(!size) {
		for(u32 i = start; i < start + digits / 2; i++) {
			char c = s.c_str[i];
			s.c_str[i] = s.c_str[start + start + digits - i - 1];
			s.c_str[start + start + digits - i - 1] = c;
		}
	}

	return idx;
}

i64 int_as_i64(void* val, _type_info* info) { PROF
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
		LOG_DEBUG_ASSERT(!"Int was not 1, 2, 4, or 8 bytes?!?!");
		return 0;
	} break;
	}
}

u32 print_int(string s, u32 idx, u8 base, void* val, _type_info* info, bool size) { PROF
	
	switch(info->size) {
	case 1: {
		if(info->_int.is_signed) {
			i8 p = *(i8*)val;
			if(p < 0) {
				idx = string_insert(s, idx, '-', size);
				return print_u64(s, idx, base, (u64)-p, size);
			} else {
				return print_u64(s, idx, base, (u64)p, size);
			}
		} else {
			return print_u64(s, idx, base, (u64)*(u8*)val, size);
		}
	} break;
	case 2: {
		if(info->_int.is_signed) {
			i16 p = *(i16*)val;
			if(p < 0) {
				idx = string_insert(s, idx, '-', size);
				return print_u64(s, idx, base, (u64)-p, size);
			} else {
				return print_u64(s, idx, base, (u64)p, size);
			}
		} else {
			return print_u64(s, idx, base, (u64)*(u16*)val, size);
		}
	} break;
	case 4: {
		if(info->_int.is_signed) {
			i32 p = *(i32*)val;
			if(p < 0) {
				idx = string_insert(s, idx, '-', size);
				return print_u64(s, idx, base, (u64)-p, size);
			} else {
				return print_u64(s, idx, base, (u64)p, size);
			}
		} else {
			return print_u64(s, idx, base, (u64)*(u32*)val, size);
		}
	} break;
	case 8: {
		if(info->_int.is_signed) {
			i64 p = *(i64*)val;
			if(p < 0) {
				idx = string_insert(s, idx, '-', size);
				return print_u64(s, idx, base, (u64)-p, size);
			} else {
				return print_u64(s, idx, base, (u64)p, size);
			}
		} else {
			return print_u64(s, idx, base, *(u64*)val, size);
		}
	} break;
	default: {
		LOG_DEBUG_ASSERT(!"Int was not 1, 2, 4, or 8 bytes?!?!");
		return idx;
	} break;
	}
}

u32 print_float(string s, u32 idx, u8 precision, void* val, _type_info* info, bool size) { PROF
	
	// this does not respect precision, but it's fine for this purpose

	f64 f = 0;
	if(info->size == 4) {
		f = (f64)*(f32*)val;
	} else {
		f = *(f64*)val;
	}
	if(f < 0) {
		idx = string_insert(s, idx, '-', size);
		f = -f;
	}

	u64 whole = (u64)f;
	idx = print_u64(s, idx, 10, whole, size);
	f64 d = f - (f64)whole;

	idx = string_insert(s, idx, '.', size);

	while(precision--) {
		d *= 10;
		u8 digit = (u8)d;
		d = d - (f64)(u64)d;

		char c = '0' + digit;
		idx = string_insert(s, idx, c, size);
	}

	return idx;
}

u32 print_enum(string s, u32 idx, void* val, _type_info* info, bool size) { PROF

	_type_info* base = TYPEINFO_H(info->_enum.base_type);

	if(!base) LOG_ERR_F("Enum % with unknown base type!", info->name);

	i64 value = int_as_i64(val, base);
	string name;

	// binary search
	u32 low = 0, high = info->_enum.member_count;
	for(;;) {

		u32 search = low + ((high - low) / 2);

		i64 mem = info->_enum.member_values[search];

		if(value == mem) {
			name = info->_enum.member_names[search];
			break;
		}

		if(mem < value) {
			low = search + 1;
		} else {
			high = search;
		}

		if(low == high) {
			break;
		}
	}

	idx = string_insert(s, idx, info->name, size);
	idx = string_insert(s, idx, string_literal("::"), size);
	idx = string_insert(s, idx, name, size);

	return idx;
}

template<typename... Targs>
string make_stringf_a(allocator* a, string fmt, Targs... args) { PROF

	string ret;
	u32 len = size_stringf(fmt, args...);
	ret 	= make_string(len, a);
	ret.len = len;
	ret.c_str[len - 1] = 0;

	_string_printf(ret, 0, fmt, false, args...);

	return ret;
}

inline bool operator==(string first, const char* second) { PROF

	return first == string_literal(second);
}

inline bool operator==(const char* first, string second) { PROF

	return string_literal(first) == second;
}

bool operator==(string first, string second) { PROF

	if(first.len != second.len) {
		return false;
	}

	for(u32 i = 0; i < first.len; i++) {
		if(first.c_str[i] != second.c_str[i]) {
			return false;
		}
	}
	
	return true;
}

bool strcmp(string first, string second) { PROF
	return first == second;
}

u32 hash_strings(string one, string two) { PROF

    u32 hash = 5381;

    for(u32 i = 0; i < one.len; i++)
        hash = ((hash << 5) + hash) ^ one.c_str[i]; /* hash * 33 + c */

    for(u32 i = 0; i < two.len; i++)
        hash = ((hash << 5) + hash) ^ two.c_str[i]; /* hash * 33 + c */

    return hash;
}

// from http://www.cse.yorku.ca/~oz/hash.html
u32 hash_string(string str) { PROF

    u32 hash = 5381;

    for(u32 i = 0; i < str.len; i++)
        hash = ((hash << 5) + hash) ^ str.c_str[i]; /* hash * 33 + c */

    return hash;
}

// adapted from http://www.json.org/JSON_checker/utf8_decode.c
u32 get_next_codepoint(string text_utf8, u32* index) { 

#ifdef MORE_PROF
	PROF
#endif

	char first, second, third, fourth;
	u32  codepoint, idx = *index;

	if(*index >= text_utf8.len) return 0;

	first = text_utf8.c_str[idx++];

	// one byte
	if((first & 0x80) == 0) {
		
		codepoint = first;

		*index += 1;
		return codepoint;
	}

	// two bytes
	if((first & 0xE0) == 0xC0) {
		
		second = text_utf8.c_str[idx++] & 0x3F;

		codepoint = ((first & 0x1F) << 6) | second;

		LOG_DEBUG_ASSERT(codepoint >= 128);

		*index += 2;
		return codepoint;
	}

	// three bytes
	if((first & 0xF0) == 0xE0) {
		
		second = text_utf8.c_str[idx++] & 0x3F;
		third  = text_utf8.c_str[idx++] & 0x3F;

		codepoint = ((first & 0x0F) << 12) | (second << 6) | third;

		LOG_DEBUG_ASSERT(codepoint >= 2048 && (codepoint < 55296 || codepoint > 57343))

		*index += 3;
		return codepoint;
	}

	// four bytes
	if((first & 0xF8) == 0xF0) {

		second = text_utf8.c_str[idx++] & 0x3F;
		third  = text_utf8.c_str[idx++] & 0x3F;
		fourth = text_utf8.c_str[idx++] & 0x3F;
		
		codepoint = ((first & 0x07) << 18) | (second << 12) | (third << 6) | fourth;

		LOG_DEBUG_ASSERT(codepoint >= 65536 && codepoint <= 1114111);

		*index += 4;
		return codepoint;
	}

	LOG_FATAL_F("Invalid codepoint index % in %", *index, text_utf8);
	return 0;
}

string make_copy_string(string src, allocator* a) { PROF

	string ret = make_string(src.cap, a);

	ret.len = src.len;

	for(u32 i = 0; i < ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

string substring(string str, u32 start, u32 end) { PROF

	string ret;

	ret.len = end - start + 1;
	ret.cap = end - start + 1;

	ret.c_str = str.c_str + start;

	return ret;
}

i32 string_last_slash(string str) { PROF

	for(u32 i = str.len - 1; i >= 0; i--) {
		if(str.c_str[i] == '\\' || str.c_str[i] == '/') {
			return i;
		}
	}

	return -1;
}

void free_string(string s, allocator* a) { PROF
	PUSH_ALLOC(a) {

		free_string(s);

	} POP_ALLOC();
}

inline string string_literal(const char* literal) { 

#ifdef MORE_PROF
	PROF
#endif

	return string_from_c_str((char*)literal);
}

string string_from_c_str(char* c_str) { 

#ifdef MORE_PROF
	PROF
#endif

	string ret;
	if(!c_str) return ret;

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);
	len++;

	ret.c_str = c_str;
	ret.len = len;
	ret.cap = len;

	return ret;
}

string make_string_from_c_str(char* c_str) { PROF

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);
	len++;

	string ret = make_string(len);
	ret.len = len;

	for(u32 i = 0; i <= len; i++) { // will copy null terminator
		ret.c_str[i] = c_str[i];
	}

	return ret;
}

void free_string(string s) { PROF

	free(s.c_str);

	s.c_str = null;
	s.cap = 0;
	s.len = 0;
}

string make_string(u32 cap, allocator* a) { PROF

	string ret;

	ret.c_str = (char*)a->allocate_(cap, a, CONTEXT);
	ret.cap = cap;

	return ret;
}

string make_string(u32 cap) { PROF

	string ret;

	ret.c_str = (char*)malloc(cap);
	ret.cap = cap;

	return ret;
}

string make_copy_string(string src) { PROF

	string ret = make_string(src.cap);

	ret.len = src.len;

	for(u32 i = 0; i < ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

// end exclusive
string make_substring(string str, u32 start, u32 end) { PROF

	string ret = make_string(end - start + 1);

	ret.len = end - start + 1;

	for(u32 i = 0, s_i = start; s_i < end; i++, s_i++) {
		ret.c_str[i] = str.c_str[s_i];
	}

	ret.c_str[ret.len - 1] = '\0';

	return ret;
}

string make_cat_strings(i32 num_strs, ...) { PROF

	va_list args;
	va_start(args, num_strs);

	array<string> params = array<string>::make(num_strs);

	for(i32 i = 0; i < num_strs; i++) {

		*params.get(i) = va_arg(args, string);
	}

	va_end(args);

	u32 len = 0;

	for(i32 i = 0; i < num_strs; i++) {

		len += params.get(i)->len - 1;
	}

	string ret = make_string(len + 1);
	ret.len = len + 1;
	u32 place = 0;

	for(i32 i = 0; i < num_strs; i++) {

		for(u32 j = 0; j < params.get(i)->len - 1; j++) {
			
			ret.c_str[place] = params.get(i)->c_str[j];
			place++;
		}
	}

	ret.c_str[place] = '\0';

	params.destroy();

	return ret;
}

string make_cat_string(string first, string second) { PROF

	string ret = make_string(first.len + second.len - 1);

	ret.len = first.len + second.len - 1;

	u32 c_i = 0;

	for(u32 i = 0; i < first.len - 1; i++, c_i++) {
		ret.c_str[c_i] = first.c_str[i];
	}

	for(u32 i = 0; i < second.len; i++, c_i++) {
		ret.c_str[c_i] = second.c_str[i];
	}

	return ret;
}
