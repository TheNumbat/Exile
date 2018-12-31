
string::operator const char*() {
	return c_str;
}

string::operator char*() {
	return c_str;
}

string::operator bool() {
	return c_str != null;
}

string operator "" _(const char* str, size_t s) {
	string ret;
	ret.c_str = (char*)str;
	ret.len = ret.cap = (u32)s + 1;
	return ret;
}

u32 string::parse_u32(u32 idx, u32* used) { 

	u32 accum = 0;
	char* place = c_str + idx;
	while(*place >= '0' && *place <= '9') {
		accum *= 10;
		accum += *place - '0';
		place++;
		if(used) (*used)++;
	}

	return accum;
}

u32 string::write(u32 idx, string ins, bool size) { 

	if(ins.len == 0) return idx;
	if(!size) _memcpy(ins.c_str, c_str + idx, ins.len - 1);
	return idx + ins.len - 1;
}

u32 string::write(u32 idx, char ins, bool size) { 

	if(!size) c_str[idx] = ins;
	return idx + 1;
}

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
inline T& get_pack_first(T& val, Targs... args) { 
	return val;
}
inline u32 get_pack_first() { 
	return 0;
}

template<typename T, typename... Targs>
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size, T val, Targs... args) { 
	return _string_printf(out, idx, fmt, size, args...);
}
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size) { 
	return _string_printf(out, idx, fmt, size);
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
u32 _string_printf(string out, u32 idx, string fmt, bool size) { 
	for(u32 i = 0; i < fmt.len - 1; i++) {
		if(fmt.c_str[i] == '%') {
			if(fmt.c_str[i + 1] == '%') {
				idx = out.write(idx, '%', size);
				i++;
			} else {
				LOG_ERR("Missing parameter for string_printf!"_);
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

u32 string::write_type(u32 idx, void* val, _type_info* info, bool size) { 
	if(info == null) {
		idx = write(idx, "UNDEF"_, size);
		return idx;
	}
	switch(info->type_type) {
	case Type::_void: {
		idx = write(idx, "void"_, size);
	} break;

	case Type::_int: {
		idx = write_int(idx, 10, val, info, size); 
	} break;

	case Type::_float: {
		idx = write_float(idx, 6, val, info, size);
	} break;

	case Type::_bool: {
		idx = write(idx, *(bool*)val ? "true"_ : "false"_, size);
	} break;

	case Type::_ptr: {
		idx = write_ptr(idx, val, info, size);
	} break;

	case Type::_array: {
		idx = write_static_array(idx, val, info, size);
	} break;

	case Type::_func: {
		idx = write(idx, info->_func.signature, size);
	} break;

	case Type::_struct: {
		idx = write_struct(idx, val, info, size);
	} break;

	case Type::_enum: {
		idx = write_enum(idx, val, info, size);
	} break;

	case Type::_string: {
		if (((string*)val)->len) {
			idx = write(idx, *(string*)val, size);
		}
	} break;
	}
	return idx;
}

u32 string::write_array(u32 idx, void* val, _type_info* info, bool size) { 

	// NOTE(max): assumes binary compatibility with vector T* first member then u32 size
	return write_vector(idx, val, info, size);
}

u32 string::write_heap(u32 idx, void* val, _type_info* info, bool size) { 

	// NOTE(max): assumes binary compatibility with vector T* first member then u32 size
	return write_vector(idx, val, info, size);
}

u32 string::write_queue(u32 idx, void* val, _type_info* info, bool size) { 

	idx = write(idx, info->name, size);

	_type_info* of = TYPEINFO_H(TYPEINFO_H(info->_struct.member_types[0])->_ptr.to);
	if(!of) {
		idx = write(idx, "[UNDEF]"_, size);
		return idx;
	}

	u32 start = *(u32*)((u8*)val + info->_struct.member_offsets[1]);
	u32 end = *(u32*)((u8*)val + info->_struct.member_offsets[2]);
	u32 capacity = *(u32*)((u8*)val + info->_struct.member_offsets[3]);
	u8* data = *(u8**)((u8*)val + info->_struct.member_offsets[0]);

	idx = write(idx, '[', size);
	if(start != UINT32_MAX) {
		u32 i = start;
		do {
			if(of->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}
			idx = write_type(idx, data + i * of->size, of, size);
			if(of->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}
			if((i + 1) % capacity != end)
				idx = write(idx, ", "_, size);

			++i %= capacity;
		} while (i != end);
	}
	idx = write(idx, ']', size);

	return idx;	
}

u32 string::write_vector(u32 idx, void* val, _type_info* info, bool size) { 

	idx = write(idx, info->name, size);

	_type_info* of = TYPEINFO_H(TYPEINFO_H(info->_struct.member_types[0])->_ptr.to);
	if(!of) {
		idx = write(idx, "[UNDEF]"_, size);
		return idx;
	}

	// NOTE(max): hard-coded order of members in vector

	u32 vec_len = *(u32*)((u8*)val + info->_struct.member_offsets[1]);
	u8* data = *(u8**)((u8*)val + info->_struct.member_offsets[0]);

	idx = write(idx, '[', size);
	for(u32 i = 0; i < vec_len; i++) {
		if(of->type_type == Type::_string) {
			idx = write(idx, '\"', size);
		}
		idx = write_type(idx, data + i * of->size, of, size);
		if(of->type_type == Type::_string) {
			idx = write(idx, '\"', size);
		}
		if(i != vec_len - 1)
			idx = write(idx, ", "_, size);
	}
	idx = write(idx, ']', size);

	return idx;
}

u32 string::write_map(u32 idx, void* val, _type_info* info, bool size) { 

	_type_info* vec_info = TYPEINFO_H(info->_struct.member_types[0]);
	u8* vec = (u8*)val + info->_struct.member_offsets[0];

	idx = write(idx, info->name, size);

	_type_info* vec_of = TYPEINFO_H(TYPEINFO_H(vec_info->_struct.member_types[0])->_ptr.to);
	if(!vec_of) {
		idx = write(idx, "[UNDEF]"_, size);
		return idx;
	}

	u32 vec_cap = *(u32*)((u8*)vec + vec_info->_struct.member_offsets[2]);
	u8* data = *(u8**)((u8*)vec + vec_info->_struct.member_offsets[0]);

	u32 number_actually_printed = 0;
	idx = write(idx, '[', size);
	for(u32 i = 0; i < vec_cap; i++) {
		if(! ((*(u32*)(data + i * vec_of->size + vec_of->_struct.member_offsets[2])) & ELEMENT_OCCUPIED_FLAG)) continue;

		if(number_actually_printed != 0)
			idx = write(idx, ", "_, size);
		number_actually_printed++;
		
		idx = write(idx, '{', size);
		{
			_type_info* key = TYPEINFO_H(vec_of->_struct.member_types[0]);
			if(key->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}
			idx = write_type(idx, data + i * vec_of->size + vec_of->_struct.member_offsets[0], key, size);
			if(key->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}
		}
		idx = write(idx, ", "_, size);
		{
			_type_info* value = TYPEINFO_H(vec_of->_struct.member_types[1]);
			if(value->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}
			idx = write_type(idx, data + i * vec_of->size + vec_of->_struct.member_offsets[1], value, size);
			if(value->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}
		}
		idx = write(idx, '}', size);
	}
	idx = write(idx, ']', size);
	return idx;
}

u32 string::write_static_array(u32 idx, void* val, _type_info* info, bool size) {  
	
	_type_info* of = TYPEINFO_H(info->_array.of);

	if (!of) {
		idx = write(idx, "UNDEF["_, size);
		idx = write_u64(idx, 10, info->_array.length, size);
		idx = write(idx, ']', size);
		return idx;
	}

	idx = write(idx, of->name, size);
	idx = write(idx, '[', size);
	for(u32 i = 0; i < info->_array.length; i++) {
		if(of->type_type == Type::_string) {
			idx = write(idx, '\"', size);
		}
		idx = write_type(idx, (u8*)val + i * of->size, of, size);
		if(of->type_type == Type::_string) {
			idx = write(idx, '\"', size);
		}
		if(i != info->_array.length - 1)
			idx = write(idx, ", "_, size);
	}

	idx = write(idx, ']', size);

	return idx;
}

u32 string::write_ptr(u32 idx, void* val, _type_info* info, bool size) {  
	idx = write(idx,"*{"_, size);
	if (info->_ptr.to == 0) {
		idx = write(idx, "UNDEF|"_, size);

		if (*(u8**)val == null) {
			idx = write(idx, "null"_, size);
		} else {
			idx = write_u64(idx, 16, (u64)(*(u8**)val), size, sizeof(void*) == 8 ? 16 : 8);
		}
	} else {
		_type_info* to = TYPEINFO_H(info->_ptr.to);
		if (*(u8**)val == null) {
			idx = write(idx, to->name, size);
			idx = write(idx, "|null"_, size);
		} else {
			if(to->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}			
			idx = write_type(idx, *(u8**)val, to, size);
			if(to->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}	
			idx = write(idx, '|', size);
			idx = write_u64(idx, 16, (u64)(*(u8**)val), size, sizeof(void*) == 8 ? 16 : 8);
		}
	}	
	idx = write(idx, '}', size);
	return idx;
}

constexpr u32 const_hash(const char* str) {

    return *str ? (u32)(*str) + 33 * const_hash(str + 1) : 5381;
}

#pragma warning(push)
#pragma warning(disable : 4307)
u32 string::write_struct(u32 idx, void* val, _type_info* info, bool size) { 

	u32 name = const_hash(info->name.c_str);

	switch(name) {
	case const_hash("v2"): {
		v2* v = (v2*)val;
		idx = write(idx, '{', size);
		idx = write_float(idx, 4, &v->x, TYPEINFO(f32), size);
		idx = write(idx, ',', size);
		idx = write_float(idx, 4, &v->y, TYPEINFO(f32), size);
		idx = write(idx, '}', size);
	} break;
	case const_hash("v3"): {
		v3* v = (v3*)val;
		idx = write(idx, '{', size);
		idx = write_float(idx, 4, &v->x, TYPEINFO(f32), size);
		idx = write(idx, ',', size);
		idx = write_float(idx, 4, &v->y, TYPEINFO(f32), size);
		idx = write(idx, ',', size);
		idx = write_float(idx, 4, &v->z, TYPEINFO(f32), size);
		idx = write(idx, '}', size);		
	} break;
	case const_hash("v4"): {
		v4* v = (v4*)val;
		idx = write(idx, '{', size);
		idx = write_float(idx, 4, &v->x, TYPEINFO(f32), size);
		idx = write(idx, ',', size);
		idx = write_float(idx, 4, &v->y, TYPEINFO(f32), size);
		idx = write(idx, ',', size);
		idx = write_float(idx, 4, &v->z, TYPEINFO(f32), size);
		idx = write(idx, ',', size);
		idx = write_float(idx, 4, &v->w, TYPEINFO(f32), size);
		idx = write(idx, '}', size);
	} break;
	case const_hash("vector"): {
		idx = write_vector(idx, val, info, size);
	} break;
	case const_hash("array"): {
		idx = write_array(idx, val, info, size);
	} break;
	case const_hash("map"): {	
		idx = write_map(idx, val, info, size);
	} break;
	case const_hash("queue"): {
		idx = write_queue(idx, val, info, size);
	} break;
	case const_hash("heap"): {
		idx = write_heap(idx, val, info, size);
	} break;
	case const_hash("_FPTR"): {
		idx = write(idx, ((_FPTR*)val)->name, size);
		idx = write(idx, "()"_, size);
	} break;
	case const_hash("func_ptr"): {
		idx = write(idx, (*(_FPTR**)val)->name, size);
		idx = write(idx, "()"_, size);
	} break;
	default: {
		idx = write_any_struct(idx, val, info, size);
	} break;
	}

	return idx;
}
#pragma warning(pop)

u32 string::write_any_struct(u32 idx, void* val, _type_info* info, bool size) {  
	idx = write(idx, info->name, size);
	idx = write(idx, "{"_, size);
	for(u32 j = 0; j < info->_struct.member_count; j++) {
		idx = write(idx, info->_struct.member_names[j], size);
		idx = write(idx, " : "_, size);

		_type_info* member = TYPEINFO_H(info->_struct.member_types[j]);
		u8* place = (u8*)val + info->_struct.member_offsets[j];

		if(member && !info->_struct.member_circular[j]) {
			if(member->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}
			idx = write_type(idx, place, member, size);
			if(member->type_type == Type::_string) {
				idx = write(idx, '\"', size);
			}
		} else {
			idx = write(idx, "UNDEF"_, size);
		}

		if(j < info->_struct.member_count - 1) {
			idx = write(idx, ", "_, size);
		}
	}
	idx = write(idx, '}', size);
	return idx;
}

u32 string::write_u64(u32 idx, u8 base, u64 val, bool size, u32 min_len) { 

	u8 digit = 0, digits = 0;
	u32 start = idx;
	if(val == 0) {
		idx = write(idx, '0', size);
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
		idx = write(idx, c, size);
	}
	while(digits < min_len) {
		digits++;
		idx = write(idx, '0', size);
	}

	if(!size) {
		for(u32 i = start; i < start + digits / 2; i++) {
			char c = c_str[i];
			c_str[i] = c_str[start + start + digits - i - 1];
			c_str[start + start + digits - i - 1] = c;
		}
	}

	return idx;
}

u32 string::write_int(u32 idx, u8 base, void* val, _type_info* info, bool size) { 
	
	switch(info->size) {
	case 1: {
		if(info->_int.is_signed) {
			i8 p = *(i8*)val;
			if(p < 0) {
				idx = write(idx, '-', size);
				return write_u64(idx, base, (u64)-p, size);
			} else {
				return write_u64(idx, base, (u64)p, size);
			}
		} else {
			return write_u64(idx, base, (u64)*(u8*)val, size);
		}
	} break;
	case 2: {
		if(info->_int.is_signed) {
			i16 p = *(i16*)val;
			if(p < 0) {
				idx = write(idx, '-', size);
				return write_u64(idx, base, (u64)-p, size);
			} else {
				return write_u64(idx, base, (u64)p, size);
			}
		} else {
			return write_u64(idx, base, (u64)*(u16*)val, size);
		}
	} break;
	case 4: {
		if(info->_int.is_signed) {
			i32 p = *(i32*)val;
			if(p < 0) {
				idx = write(idx, '-', size);
				return write_u64(idx, base, (u64)-p, size);
			} else {
				return write_u64(idx, base, (u64)p, size);
			}
		} else {
			return write_u64(idx, base, (u64)*(u32*)val, size);
		}
	} break;
	case 8: {
		if(info->_int.is_signed) {
			i64 p = *(i64*)val;
			if(p < 0) {
				idx = write(idx, '-', size);
				return write_u64(idx, base, (u64)-p, size);
			} else {
				return write_u64(idx, base, (u64)p, size);
			}
		} else {
			return write_u64(idx, base, *(u64*)val, size);
		}
	} break;
	default: {
		LOG_DEBUG_ASSERT(!"Int was not 1, 2, 4, or 8 bytes?!?!");
		return idx;
	} break;
	}
}

u32 string::write_float(u32 idx, u8 precision, void* val, _type_info* info, bool size) { 
	
	// this does not respect precision, but it's fine for this purpose

	f64 f = 0;
	if(info->size == 4) {
		f = (f64)*(f32*)val;
	} else {
		f = *(f64*)val;
	}
	if(f < 0) {
		idx = write(idx, '-', size);
		f = -f;
	}

	u64 whole = (u64)f;
	idx = write_u64(idx, 10, whole, size);
	f64 d = f - (f64)whole;

	idx = write(idx, '.', size);

	while(precision--) {
		d *= 10;
		u8 digit = (u8)d;
		d = d - (f64)(u64)d;

		char c = '0' + digit;
		idx = write(idx, c, size);
	}

	return idx;
}

u32 string::write_enum(u32 idx, void* val, _type_info* info, bool size) { 

	_type_info* base = TYPEINFO_H(info->_enum.base_type);

	if(!base) LOG_ERR_F("Enum % with unknown base type!"_, info->name);

	i64 value = int_as_i64(val, base);
	
	string name = enum_to_string(value, info);

	idx = write(idx, info->name, size);
	idx = write(idx, "::"_, size);
	idx = write(idx, name, size);

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

inline bool operator==(string first, const char* second) { 

	return first == string::literal(second);
}

inline bool operator==(const char* first, string second) { 

	return string::literal(first) == second;
}

bool operator==(string first, string second) { 

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

bool strcmp(string first, string second) { 
	return first == second;
}

bool operator<=(string first, string second) { 

	if(first == second) return true;

	for(u32 i = 0; i < first.len && i < second.len; i++) {
		if(first.c_str[i] < second.c_str[i]) {
			return true;
		} else if(first.c_str[i] == second.c_str[i]) {
			continue;
		}
		return false;
	}
	return true;
}

inline u32 hash(string one, string two) { 

    u32 hash = 5381;

    for(u32 i = 0; i < one.len; i++)
        hash = ((hash << 5) + hash) ^ one.c_str[i]; /* hash * 33 + c */

    for(u32 i = 0; i < two.len; i++)
        hash = ((hash << 5) + hash) ^ two.c_str[i]; /* hash * 33 + c */

    return hash;
}

// from http://www.cse.yorku.ca/~oz/hash.html
inline u32 hash(string str) { 

    u32 hash = 5381;

    for(u32 i = 0; i < str.len; i++)
        hash = ((hash << 5) + hash) ^ str.c_str[i]; /* hash * 33 + c */

    return hash;
}

#if 0
// adapted from https://github.com/skeeto/branchless-utf8/blob/master/utf8.h
u32 string::get_next_codepoint(u32* index) { 

	static const char lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };
    static const i32 masks[]  = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
    static const u32 mins[] = {4194304, 0, 128, 2048, 65536};
    static const i32 shiftc[] = {0, 18, 12, 6, 0};
    static const i32 shifte[] = {0, 6, 4, 2, 0};

    u8 *s = (u8*)c_str + *index;
    i32 l = lengths[s[0] >> 3];

    *index += l + !l;

    u32 c = 0;
    c  = (u32)(s[0] & masks[l]) << 18;
    c |= (u32)(s[1] & 0x3f) << 12;
    c |= (u32)(s[2] & 0x3f) <<  6;
    c |= (u32)(s[3] & 0x3f) <<  0;
    c >>= shiftc[l];

#if 1
    u32 e = 0;
    e  = (c < mins[l]) << 6; // non-canonical encoding
    e |= ((c >> 11) == 0x1b) << 7;  // surrogate half?
    e |= (c > 0x10FFFF) << 8;  // out of range?
    e |= (s[1] & 0xc0) >> 2;
    e |= (s[2] & 0xc0) >> 4;
    e |= (s[3]       ) >> 6;
    e ^= 0x2a; // top two bits of each tail byte correct?
    e >>= shifte[l];
    LOG_ASSERT(!e);
#endif

    return c;
}
#else
// adapted from http://www.json.org/JSON_checker/utf8_decode.c
u32 string::get_next_codepoint(u32* index) { 

	char first, second, third, fourth;
	u32  codepoint, idx = *index;

	if(*index >= len) return 0;

	first = c_str[idx++];

	// one byte
	if((first & 0x80) == 0) {
		
		codepoint = first;

		*index += 1;
		return codepoint;
	}

	// two bytes
	if((first & 0xE0) == 0xC0) {
		
		second = c_str[idx++] & 0x3F;

		codepoint = ((first & 0x1F) << 6) | second;

		LOG_DEBUG_ASSERT(codepoint >= 128);

		*index += 2;
		return codepoint;
	}

	// three bytes
	if((first & 0xF0) == 0xE0) {
		
		second = c_str[idx++] & 0x3F;
		third  = c_str[idx++] & 0x3F;

		codepoint = ((first & 0x0F) << 12) | (second << 6) | third;

		LOG_DEBUG_ASSERT(codepoint >= 2048 && (codepoint < 55296 || codepoint > 57343));

		*index += 3;
		return codepoint;
	}

	// four bytes
	if((first & 0xF8) == 0xF0) {

		second = c_str[idx++] & 0x3F;
		third  = c_str[idx++] & 0x3F;
		fourth = c_str[idx++] & 0x3F;
		
		codepoint = ((first & 0x07) << 18) | (second << 12) | (third << 6) | fourth;

		LOG_DEBUG_ASSERT(codepoint >= 65536 && codepoint <= 1114111);

		*index += 4;
		return codepoint;
	}

	LOG_FATAL_F("Invalid codepoint index % in %"_, *index, *this);
	return 0;
}
#endif

string string::make_copy(string src, allocator* a) { 

	string ret = string::make(src.len, a);

	ret.len = src.len;

	for(u32 i = 0; i < ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

string string::make_copy_plt(string src) { 

	string ret; 
	ret.len = src.len;
	ret.cap = src.cap;
	ret.c_str = (char*)global_api->heap_alloc(ret.cap);

	for(u32 i = 0; i < ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

string string::substring(u32 start, u32 end) { 

	string ret;

	ret.len = end - start + 1;
	ret.cap = end - start + 1;

	ret.c_str = c_str + start;

	return ret;
}

i32 string::first_slash() { 

	for(u32 i = 0; i < len; i++) {
		if(c_str[i] == '\\' || c_str[i] == '/') {
			return i;
		}
	}

	return -1;
}

i32 string::last_slash() { 

	for(u32 i = len - 1; i >= 0; i--) {
		if(c_str[i] == '\\' || c_str[i] == '/') {
			return i;
		}
	}

	return -1;
}

string string::trim_no_term() { 

	u32 beg = 0, end = len - 2;
	for(; beg < len && whitespace(c_str[beg]); beg++);
	for(; end >= 0 && whitespace(c_str[end]); end--);

	return substring(beg, end + 1);
}

void string::destroy(allocator* a) { 

	a->free_(c_str, cap, a, CONTEXT);

	c_str = null;
	cap = 0;
	len = 0;
}

bool whitespace(char c) { 

	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

char uppercase(char c) { 

	if(c >= 'a' && c <= 'z') return c - 'a' + 'A';
	return c;
}

bool string::starts_with_insensitive(string prefix) { 

	for(u32 i = 0; i < prefix.len; i++) {
		if(uppercase(c_str[i]) != uppercase(prefix.c_str[i])) return false;
	}
	return true;
}

string string::first_word_no_term() { 

	u32 i = 0;
	for(; i < len; i++)
		if(c_str[i] == ' ') break;

	if(i == len) 
		return substring(0, len - 2);

	return substring(0, i > 0 ? i - 1 : 0);
}

string string::trim_first_word() { 

	u32 i = 0;
	for(; i < len; i++)
		if(c_str[i] == ' ') break;

	return substring(i < len ? i + 1 : i, len - 1);
}

string string::literal(const char* literal) { 

	return string::from_c_str((char*)literal);
}

string string::from_c_str(char* begin, char* end) { 

	string ret;

	ret.c_str = begin;
	ret.len = (u32)(end - begin);
	ret.cap = (u32)(end - begin);

	return ret;
}

string string::from_c_str(char* c_str) { 

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

string string::make_from_c_str(char* c_str) { 

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);
	len++;

	string ret = string::make(len);
	ret.len = len;

	for(u32 i = 0; i <= len; i++) { // will copy null terminator
		ret.c_str[i] = c_str[i];
	}

	return ret;
}

void string::destroy() { 

	free(c_str, cap);

	c_str = null;
	cap = 0;
	len = 0;
}


string string::make(u32 cap, allocator* a) { 

	string ret;

	ret.c_str = (char*)a->allocate_((u64)cap, 0, a, CONTEXT);
	ret.cap = cap;

	return ret;
}

string string::make(u32 cap) { 

	string ret;

	ret.c_str = (char*)malloc(cap);
	ret.cap = cap;

	return ret;
}

string string::make_copy(string src) { 

	string ret = string::make(src.len);

	ret.len = src.len;

	for(u32 i = 0; i < ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

// end exclusive
string string::make_substring(string str, u32 start, u32 end) { 

	string ret = string::make(end - start + 1);

	ret.len = end - start + 1;

	for(u32 i = 0, s_i = start; s_i < end; i++, s_i++) {
		ret.c_str[i] = str.c_str[s_i];
	}

	ret.c_str[ret.len - 1] = '\0';

	return ret;
}

string string::make_cat_v(i32 num_strs, ...) { 

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

	string ret = string::make(len + 1);
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

string string::make_cat(string first, string second) { 

	string ret = string::make(first.len + second.len - 1);

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

string string::make_from_c_str_plt(char* c_str) {

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);
	len++;

	string ret;
	ret.c_str = (char*)global_api->heap_alloc(len);
	ret.cap = len;
	ret.len = len;

	for(u32 i = 0; i <= len; i++) { // will copy null terminator
		ret.c_str[i] = c_str[i];
	}

	return ret;
}

void string::destroy_plt() {

	global_api->heap_free(c_str);

	c_str = null;
	cap = 0;
	len = 0;
}
