
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

u32 hash_strings(string one, string two) {

    u32 hash = 5381;

    for(u32 i = 0; i < one.len; i++)
        hash = ((hash << 5) + hash) ^ one.c_str[i]; /* hash * 33 + c */

    for(u32 i = 0; i < two.len; i++)
        hash = ((hash << 5) + hash) ^ two.c_str[i]; /* hash * 33 + c */

    return hash;
}

// from http://www.cse.yorku.ca/~oz/hash.html
u32 hash_string(string str) {

    u32 hash = 5381;

    for(u32 i = 0; i < str.len; i++)
        hash = ((hash << 5) + hash) ^ str.c_str[i]; /* hash * 33 + c */

    return hash;
}

// adapted from http://www.json.org/JSON_checker/utf8_decode.c
u32 get_next_codepoint(string text_utf8, u32* index) {

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

	LOG_DEBUG_ASSERT(false);
	return 0;
}

string make_copy_string(string src, allocator* a) {

	string ret = make_string(src.cap, a);

	ret.len = src.len;

	for(u32 i = 0; i < ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

string substring(string str, u32 start, u32 end) {

	string ret;

	ret.len = end - start + 1;
	ret.cap = end - start + 1;

	ret.c_str = str.c_str + start;

	return ret;
}


i32 string_last_slash(string str) {

	for(u32 i = str.len; i >= 0; i--) {
		if(str.c_str[i] == '\\' || str.c_str[i] == '/') {
			return i;
		}
	}

	return -1;
}

void free_string(string s, allocator* a) {
	PUSH_ALLOC(a) {

		free_string(s);

	} POP_ALLOC();
}

string make_stringf_a(allocator* a, string fmt, ...) {
	
	string ret;

	PUSH_ALLOC(a) {

		va_list args;
		va_start(args, fmt);
		ret = make_vstringf(fmt, args);
		va_end(args);

	} POP_ALLOC();

	return ret;
}

string make_stringf(string fmt, ...) {

	va_list args;
	va_start(args, fmt);

	string ret = make_vstringf(fmt, args);

	va_end(args);

	return ret;
}

string make_vstringf_a(allocator* a, string fmt, va_list args) {

	string ret;

	PUSH_ALLOC(a) {

		ret = make_vstringf(fmt, args);

	} POP_ALLOC();

	return ret;	
}

#include <cstdio>
#pragma warning(push)
#pragma warning(disable : 4996)
string make_vstringf(string fmt, va_list args) {

	i32 len = _vscprintf(fmt.c_str, args) + 1;

	string ret = make_string(len);
	ret.len = len;

	vsnprintf(ret.c_str, ret.len, fmt.c_str, args);

	return ret;
}
#pragma warning(pop)

string string_literal(const char* literal) {

	return string_from_c_str((char*)literal);
}

string string_from_c_str(char* c_str) {

	string ret;

	u32 len;
	for(len = 0; c_str[len] != '\0'; len++);
	len++;

	ret.c_str = c_str;
	ret.len = len;
	ret.cap = len;

	return ret;
}

string make_string_from_c_str(char* c_str) {

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

void free_string(string s) {

	free(s.c_str);

	s.c_str = NULL;
	s.cap = 0;
	s.len = 0;
}

string make_string(u32 cap, allocator* a) {

	string ret;

	ret.c_str = (char*)a->allocate_(cap, a, CONTEXT);
	ret.cap = cap;

	return ret;
}

string make_string(u32 cap) {

	string ret;

	ret.c_str = (char*)malloc(cap);
	ret.cap = cap;

	return ret;
}

string make_copy_string(string src) {

	string ret = make_string(src.cap);

	ret.len = src.len;

	for(u32 i = 0; i < ret.len; i++) { // will copy null terminator
		ret.c_str[i] = src.c_str[i];
	}

	return ret;
}

// end inclusive
string make_substring(string str, u32 start, u32 end) {

	string ret = make_string(end - start + 1);

	ret.len = end - start + 1;

	for(u32 i = 0, s_i = start; s_i <= end; i++, s_i++) {
		ret.c_str[i] = str.c_str[s_i];
	}

	return ret;
}

string make_cat_strings(i32 num_strs, ...) {

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

string make_cat_string(string first, string second) {

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
