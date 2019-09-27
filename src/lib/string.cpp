
#include "string.h"
#include "log.h"
#include "alloc.h"
#include <string.h>

thread_local char g_scratch_buf_underlying[1024];
thread_local string g_scratch_buf = string::from(g_scratch_buf_underlying, 1024);

string::string() {}

string::string(const char* lit) {
    *this = literal(lit);
}

string::operator const char*() {
    return c_str;
}

string::operator char*() {
    return c_str;
}

string string::make(u32 cap) {
    string ret;
    ret.c_str = (char*)galloc(cap);
    ret.cap = cap;
    return ret;
}

string string::copy(string src) {
    string ret;
    ret.c_str = (char*)galloc(src.cap);
    ret.cap = src.cap;
    ret.len = src.len;
    memcpy(ret.c_str, src.c_str, src.len);
    return ret;
}

string string::take(string& src) {
    string ret = src;
    src.c_str = null;
    src.cap = src.len = 0;
    return ret;
}

string string::from(const char* arr, u32 cap) {
    string ret;
    ret.c_str = (char*)arr;
    ret.cap = cap;
    return ret;
}

string string::literal(const char* lit) {
    string ret;
    ret.c_str = (char*)lit;
    ret.cap = ret.len = strlen(lit) + 1;
    return ret;
}

void string::destroy() {
    gfree(c_str);
    c_str = null;
    len = cap = 0;
}

char* string::begin() const {
    return c_str;
}

char* string::end() const {
    if(len) return c_str + len - 1;
    else return c_str;
}

char string::operator[](u32 idx) const {
    assert(idx < len);
    return c_str[idx];
}

char& string::operator[](u32 idx) {
    assert(idx < len);
    return c_str[idx];
}

const string string::sub_end(u32 s) const {
    assert(s <= len);
    string ret;
    ret.c_str = c_str + s;
    ret.cap = cap - s;
    ret.len = len - s;
    return ret;
}

u32 string::write(u32 idx, string cpy) {
    assert(cap && idx + cpy.len - 1 < len);
    memcpy(c_str + idx, cpy.c_str, cpy.len - 1);
    return cpy.len - 1;
}

u32 string::write(u32 idx, char cpy) {
    assert(cap && idx < len);
    c_str[idx] = cpy;
    return 1;
}

string last_file(string path) {
    u32 loc = path.len - 1;
    for(; loc >= 0; loc--) {
		if(path.c_str[loc] == '\\' || path.c_str[loc] == '/') {
			break;
		}
	}
    return path.sub_end(loc + 1);
}
