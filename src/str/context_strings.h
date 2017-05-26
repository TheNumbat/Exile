
#pragma once

string make_string_from_c_str(char* c_str);
void free_string(string s);
string make_string(u32 cap);
string make_copy_string(string src);
string make_substring(string str, u32 start, u32 end);
string make_cat_string(string first, string second);

