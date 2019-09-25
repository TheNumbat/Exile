
#include "string.h"
#include "log.h"
#include "alloc.h"
#include <string.h>

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

string string::literal(const char* lit) {
    string ret;
    ret.c_str = (char*)lit;
    ret.cap = ret.len = strlen(lit);
    return ret;
}

void string::destroy() {
    gfree(c_str);
    c_str = null;
    len = cap = 0;
}

string string::sub_end(u32 s) {
    assert(s < len);
    string ret;
    ret.c_str = c_str + s;
    ret.cap = cap - s;
    ret.len = len - s;
    return ret;
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
