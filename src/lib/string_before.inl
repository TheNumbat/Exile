#ifndef RUNNING_META
template<typename A>
#endif
struct norefl astring {
	char* c_str = null;
	u32 cap	    = 0;
	u32 len		= 0;

    astring() {}
    astring(const char* lit) {*this = literal(lit);}

	static astring make(u32 cap) {
		astring ret;
    	ret.c_str = A::template alloc<char*>(cap);
    	ret.cap = cap;
    	return ret;
	}
	static astring copy(astring src) {
		astring ret;
		ret.c_str = A::template alloc<char*>(src.cap);
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
	static astring literal(const char* lit) {
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

	char operator[](u32 idx) const;
	char& operator[](u32 idx);

	char* begin() {return c_str;}
	char* end() {
		if(len) return c_str + len - 1;
		return c_str;
	}

	const astring sub_end(u32 s) const;
	
	u32 write(u32 idx, astring cpy);
	u32 write(u32 idx, char cpy);
};

static constexpr char Mdefault_name[] = "Mdefault";
using Mdefault = Mallocator<Mdefault_name>;

using string = astring<Mdefault>;

inline thread_local char g_scratch_buf_underlying[1024];
inline thread_local string g_scratch_buf = string::from(g_scratch_buf_underlying, 1024);

string last_file(string path) {
    u32 loc = path.len - 1;
    for(; loc >= 0; loc--) {
		if(path.c_str[loc] == '\\' || path.c_str[loc] == '/') {
			break;
		}
	}
    return path.sub_end(loc + 1);
}