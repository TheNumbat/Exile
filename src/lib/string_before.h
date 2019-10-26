
#pragma once

#include <string.h>

template<typename A>
struct astring;

// Non-modifiable string, kind of like a string_view.
// Should be used to represent string literals or more generally views into
// memory that is not owned by this string.
template<>
struct astring<void> {
    char* c_str = null;
    u32 cap	  = 0;
    u32 len     = 0;

    astring() {}
    astring(const char* lit) {
        c_str  = (char*)lit;
        cap = strlen(lit) + 1;
        len = cap;
    }
    astring(const char* arr, u32 c) {
        c_str  = (char*)arr;
        cap = c;
        len = c;
    }

    static astring literal(const char* lit) {
        return {lit};
    }
    static astring from(const char* arr, u32 cap) {
        return {arr, cap};
    }

    operator const char*() {return c_str;}

    char operator[](u32 idx) const;
    astring sub_end(u32 s) const;

    const char* begin() {return c_str;}
    const char* end() {
        if(cap) return c_str + cap - 1;
        return c_str;
    }
};

template<typename A>
struct astring {
    char* c_str = null;
    u32 cap	    = 0;
    u32 len		= 0;

    astring() {}
    astring(const char* lit) {*this = from_c(lit);}

    static astring make(u32 cap) {
        astring ret;
        ret.c_str = A::template alloc<char>(cap);
        ret.cap = cap;
        return ret;
    }
    static astring copy(astring src) {
        astring ret;
        ret.c_str = A::template alloc<char>(src.cap);
        ret.cap = src.cap;
        ret.len = src.len;
        memcpy(ret.c_str, src.c_str, src.len);
        return ret;
    }
    static astring take(astring& src) {
        astring ret = src;
        src.c_str = null;
        src.cap = src.len = 0;
        return ret;
    }
    static astring from_c(const char* lit) {
        astring ret;
        ret.c_str = (char*)lit;
        ret.cap = ret.len = strlen(lit) + 1;
        return ret;
    }
    static astring from(const char* arr, u32 cap) {
        astring ret;
        ret.c_str = (char*)arr;
        ret.cap = cap;
        return ret;
    }
    void destroy() {
        A::dealloc(c_str);
        c_str = null;
        len = cap = 0;
    }

    operator const char*() {return c_str;}
    operator char*() {return c_str;}
    
    astring<void> view() const {
        return {c_str, cap, len};
    }

    char operator[](u32 idx) const;
    char& operator[](u32 idx);

    char* begin() {return c_str;}
    char* end() {
        if(len) return c_str + len - 1;
        return c_str;
    }

    const astring sub_end(u32 s) const;
    
    template<typename SA>
    u32 write(u32 idx, astring<SA> cpy);
    u32 write(u32 idx, astring<void> cpy);
    u32 write(u32 idx, char cpy);
};

using string = astring<Mdefault>;
using literal = astring<void>;

static thread_local char g_scratch_buf_underlying[4096];
static thread_local string g_scratch_buf = string::from(g_scratch_buf_underlying, 4096);

template<typename L, typename R>
bool operator==(astring<L> l, astring<R> r) {
    return strcmp(l.c_str, r.c_str) == 0;
}

template<typename A>
astring<A> last_file(astring<A> path);

