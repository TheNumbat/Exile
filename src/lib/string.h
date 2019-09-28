
#pragma once

#include "basic.h"

struct norefl string {
	char* c_str = null;
	u32 cap	    = 0;
	u32 len		= 0;

    string();
    string(const char* lit);

	// allocate
	static string make(u32 cap);
	static string copy(string src);
	void destroy();
    
	// no allocate
	static string take(string& src);
	static string literal(const char* lit);
	static string from(const char* arr, u32 cap);

	operator const char*();
	operator char*();

	char operator[](u32 idx) const;
	char& operator[](u32 idx);

	char* begin() const;
	char* end() const;

	const string sub_end(u32 s) const;
	
	u32 write(u32 idx, string cpy);
	u32 write(u32 idx, char cpy);
};

extern thread_local string g_scratch_buf;

string last_file(string path);


