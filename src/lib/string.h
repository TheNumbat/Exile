
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

	operator const char*();
	operator char*();

	string sub_end(u32 s);
};

string last_file(string path);

