
u32 parse_u32(string s, u32 idx, u32* used) {

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

u32 string_insert(string s, u32 idx, string ins, bool size) { FUNC
	if(!size) memcpy(ins.c_str, s.c_str + idx, ins.len - 1);
	return idx + ins.len - 1;
}

u32 print_int(string s, u32 idx, i32 i, bool size) { FUNC
	return idx;
}

u32 print_float(string s, u32 idx, f32 i, bool size) { FUNC
	return idx;
}

template<typename T>
u32 print_type(string s, u32 idx, T& val, _type_info* info, bool size) { FUNC
	if(info == NULL) {
		info = TYPEINFO(T);
		if(!info) {
			idx = string_insert(s, idx, string_literal("UNDEF"), size);
			return idx;
		}
	}
	switch(info->type_type) {
	case Type::_void: {
		idx = string_insert(s, idx, string_literal("void"), size);
	} break;
	case Type::_int: {
		idx = print_int(s, idx, *(i32*)&val); // need bit size/signed flags
	} break;
	case Type::_float: {
		idx = print_float(s, idx, *(float*)&val);		// need bit size flags
	} break;
	case Type::_bool: {
		idx = string_insert(s, idx, string_literal(*(bool*)&val ? "true" : "false"), size);
	} break;
	case Type::_ptr: {
		idx = string_insert(s, idx, string_literal("*{"), size);
		if(*(u8**)&val == NULL) {
			idx = string_insert(s, idx, info->_ptr.to->name, size);
			idx = string_insert(s, idx, string_literal("|NULL"), size);
		} else if(info->_ptr.to == NULL) {
			idx = string_insert(s, idx, string_literal("UNDEF"), size);
		} else {
			idx = print_type(s, idx, **(u8**)&val, info->_ptr.to, size);
		}
		idx = string_insert(s, idx, string_literal("}"), size);
	} break;
	case Type::_func: {
	} break;
	case Type::_struct: {
		idx = string_insert(s, idx, info->name, size);
		idx = string_insert(s, idx, string_literal("{"), size);
		for(u32 j = 0; j < info->_struct.member_count; j++) {
			idx = string_insert(s, idx, info->_struct.member_names[j], size);
			idx = string_insert(s, idx, string_literal(" : "), size);

			_type_info* member = info->_struct.member_types[j];
			u8* place = (u8*)&val + info->_struct.member_offsets[j];

			if(member->type_type == Type::_string) {
				idx = string_insert(s, idx, string_literal("\""), size);
			}
			idx = print_type(s, idx, *place, member, size);
			if(member->type_type == Type::_string) {
				idx = string_insert(s, idx, string_literal("\""), size);
			}

			if(j < info->_struct.member_count - 1) {
				idx = string_insert(s, idx, string_literal(", "), size);
			}
		}
		idx = string_insert(s, idx, string_literal("}"), size);
	} break;
	case Type::_enum: {
	} break;
	case Type::_string: {
		if (((string*)&val)->len) {
			idx = string_insert(s, idx, *(string*)&val, size);
		}
	} break;
	}
	return idx;
}

inline u32 get_pack_first() { FUNC
	return 0;
}
template<typename T, typename... Targs> 
inline T get_pack_first(T& val, Targs... args) { FUNC
	return val;
}

template<typename T, typename... Targs>
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size, T val, Targs... args) { FUNC
	return _string_printf(out, idx, fmt, size, args...);
}
inline u32 _string_printf_fwd(string out, u32 idx, string fmt, bool size) { FUNC
	return _string_printf(out, idx, fmt, size);
}

u32 _string_printf(string out, u32 idx, string fmt, bool size) { FUNC
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
	if(!size) {
		out.c_str[idx] = '\0';
	}
	idx++;
	return idx;
}
template<typename T, typename... Targs>
u32 _string_printf(string out, u32 idx, string fmt, bool size, T value, Targs... args) { FUNC

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

					if(fmt.c_str[i + 1] == '*') {	// total length passed as a parameter
						pad = *(u32*)&value;			
						i++;
						len_in_param = true;

						auto param = get_pack_first(args...);
						u32 tmp_idx = print_type(out, idx, param, NULL, size);
						sized = tmp_idx - idx;
						idx = tmp_idx;
					} else {	// total length passed in format string
						u32 len = 0;
						i++;
						pad = parse_u32(fmt, i, &len);
						i += len;
						u32 tmp_idx = print_type(out, idx, value, NULL, size);
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
						sized = print_type(out, idx, param, NULL, true) - idx;
					} else {	// total length passed in format string
						u32 len = 0;
						i++;
						pad = parse_u32(fmt, i, &len);
						i += len;
						sized = print_type(out, idx, value, NULL, true) - idx;
					}

					while(sized < pad) {
						idx = string_insert(out, idx, string_literal(" "), size);
						sized++;
					}
					
					if (len_in_param) {
						auto param = get_pack_first(args...);
						idx = print_type(out, idx, param, NULL, size);
						return _string_printf_fwd(out, idx, string_from_c_str(fmt.c_str + i + 1), size, args...);;
					} else {
						idx = print_type(out, idx, value, NULL, size);
						return _string_printf(out, idx, string_from_c_str(fmt.c_str + i), size, args...);
					}
				} else {
					idx = print_type(out, idx, value, NULL, size);
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
	if(!size) {
		out.c_str[idx] = '\0';
	}
	idx++;
	return idx;
}

template<typename... Targs>
void string_printf(string out, string fmt, Targs... args) { FUNC
	_string_printf(out, 0, fmt, false, args...);
}

template<typename... Targs>
string make_stringf(string fmt, Targs... args) { FUNC

	string ret;
	u32 len = _string_printf(ret, 0, fmt, true, args...);
	ret 	= make_string(len);
	ret.len = len;

	_string_printf(ret, 0, fmt, false, args...);

	return ret;
}

template<typename... Targs>
string make_stringf_a(allocator* a, string fmt, Targs... args) { FUNC

	string ret;
	u32 len = _string_printf(ret, 0, fmt, true, args...);
	ret 	= make_string(len, a);
	ret.len = len;

	_string_printf(ret, 0, fmt, false, args...);

	return ret;
}

bool operator==(string first, string second) { FUNC

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

bool strcmp(string first, string second) { FUNC
	return first == second;
}

u32 hash_strings(string one, string two) { FUNC

    u32 hash = 5381;

    for(u32 i = 0; i < one.len; i++)
        hash = ((hash << 5) + hash) ^ one.c_str[i]; /* hash * 33 + c */

    for(u32 i = 0; i < two.len; i++)
        hash = ((hash << 5) + hash) ^ two.c_str[i]; /* hash * 33 + c */

    return hash;
}

// from http://www.cse.yorku.ca/~oz/hash.html
u32 hash_string(string str) { FUNC

    u32 hash = 5381;

    for(u32 i = 0; i < str.len; i++)
        hash = ((hash << 5) + hash) ^ str.c_str[i]; /* hash * 33 + c */

    return hash;
}

// adapted from http://www.json.org/JSON_checker/utf8_decode.c
u32 get_next_codepoint(string text_utf8, u32* index) { FUNC

	char first, second, third, fourth;
	u32 codepoint;

	if(*index >= text_utf8.len) return 0;

	first = text_utf8.c_str[(*index)++];

	// one byte
	if((first & 0x80) == 0) {
		
		codepoint = first;

		return codepoint;
	}

	// two bytes
	if((first & 0xE0) == 0xC0) {
		
		second = text_utf8.c_str[(*index)++] & 0x3F;

		codepoint = ((first & 0x1F) << 6) | second;

		LOG_DEBUG_ASSERT(codepoint >= 128);

		return codepoint;
	}

	// three bytes
	if((first & 0xF0) == 0xE0) {
		
		second = text_utf8.c_str[(*index)++] & 0x3F;
		third  = text_utf8.c_str[(*index)++] & 0x3F;

		codepoint = ((first & 0x0F) << 12) | (second << 6) | third;

		LOG_DEBUG_ASSERT(codepoint >= 2048 && (codepoint < 55296 || codepoint > 57343))

		return codepoint;
	}

	// four bytes
	if((first & 0xF8) == 0xF0) {

		second = text_utf8.c_str[(*index)++] & 0x3F;
		third  = text_utf8.c_str[(*index)++] & 0x3F;
		fourth = text_utf8.c_str[(*index)++] & 0x3F;
		
		codepoint = ((first & 0x07) << 18) | (second << 12) | (third << 6) | fourth;

		LOG_DEBUG_ASSERT(codepoint >= 65536 && codepoint <= 1114111);

		return codepoint;
	}

	LOG_DEBUG_ASSERT(!"Invalid codepoint!");
	return 0;
}

string make_copy_string(string src, allocator* a) { FUNC

	string ret = make_string(src.cap, a);

	ret.len = src.len;

	for(u32 i = 0; i < ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

string substring(string str, u32 start, u32 end) { FUNC

	string ret;

	ret.len = end - start + 1;
	ret.cap = end - start + 1;

	ret.c_str = str.c_str + start;

	return ret;
}

i32 string_last_slash(string str) { FUNC

	for(u32 i = str.len; i >= 0; i--) {
		if(str.c_str[i] == '\\' || str.c_str[i] == '/') {
			return i;
		}
	}

	return -1;
}

void free_string(string s, allocator* a) { FUNC
	PUSH_ALLOC(a) {

		free_string(s);

	} POP_ALLOC();
}

string string_literal(const char* literal) { FUNC

	return string_from_c_str((char*)literal);
}

string string_from_c_str(char* c_str) { FUNC

	string ret;

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);
	len++;

	ret.c_str = c_str;
	ret.len = len;
	ret.cap = len;

	return ret;
}

string make_string_from_c_str(char* c_str) { FUNC

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

void free_string(string s) { FUNC

	free(s.c_str);

	s.c_str = NULL;
	s.cap = 0;
	s.len = 0;
}

string make_string(u32 cap, allocator* a) { FUNC

	string ret;

	ret.c_str = (char*)a->allocate_(cap, a, CONTEXT);
	ret.cap = cap;

	return ret;
}

string make_string(u32 cap) { FUNC

	string ret;

	ret.c_str = (char*)malloc(cap);
	ret.cap = cap;

	return ret;
}

string make_copy_string(string src) { FUNC

	string ret = make_string(src.cap);

	ret.len = src.len;

	for(u32 i = 0; i < ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

// end inclusive
string make_substring(string str, u32 start, u32 end) { FUNC

	string ret = make_string(end - start + 1);

	ret.len = end - start + 1;

	for(u32 i = 0, s_i = start; s_i <= end; i++, s_i++) {
		ret.c_str[i] = str.c_str[s_i];
	}

	return ret;
}

string make_cat_strings(i32 num_strs, ...) { FUNC

	va_list args;
	va_start(args, num_strs);

	array<string> params = make_array<string>(num_strs);

	for(i32 i = 0; i < num_strs; i++) {

		*array_get(&params, i) = va_arg(args, string);
	}

	va_end(args);

	u32 len = 0;

	for(i32 i = 0; i < num_strs; i++) {

		len += array_get(&params, i)->len - 1;
	}

	string ret = make_string(len + 1);
	ret.len = len + 1;
	u32 place = 0;

	for(i32 i = 0; i < num_strs; i++) {

		for(u32 j = 0; j < array_get(&params, i)->len - 1; j++) {
			
			ret.c_str[place] = array_get(&params, i)->c_str[j];
			place++;
		}
	}

	ret.c_str[place] = '\0';

	destroy_array(&params);

	return ret;
}

string make_cat_string(string first, string second) { FUNC

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
