
// don't care about using libraries in this; it's just to build the meta-program

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4505)
#pragma warning(disable : 4456)

#include "basic_types.h"

#include <4cpp/4cpp_lexer.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>

using namespace std;

/*
Actual RTTI generated at compile time (but structured at runtime?)
*/

struct _struct {
    string name;
    vector<string> mem_types;
    vector<string> mem_names;
    bool templated;
};

struct _token {
	_token(Cpp_Token t, char* data) : str(data + t.start, t.size) {
		token = t;
	}
	Cpp_Token token;
	string str;

};
map<string, _struct> _structs;

set<string>    files;
vector<_token> all_tokens;
vector<_token> lib_includes;
string path;
ofstream fout;

void struct_out(const _struct& s) {
    if(!s.templated) {
        fout << endl
             << "\t{" << endl
             << "\t\t// " << s.name << endl
             << "\t\t_type_info " << s.name << "_t;" << endl
             << "\t\t" << s.name << "_t.type_type = Type::_struct;" << endl
             << "\t\t" << s.name << "_t.size = sizeof(" << s.name << ");" << endl;

        for(i32 i = 0; i < s.mem_types.size(); i++) {
            fout << "\t\t" << s.name << "_t._struct.member_types[" << i << "] = TYPEINFO("
                 << s.mem_types[i] << ");" << endl;
        }
        for(i32 i = 0; i < s.mem_names.size(); i++) {
            fout << "\t\t" << s.name << "_t._struct.member_names[" << i << "] = string_literal(\""
                 << s.mem_names[i] << "\");" << endl;
        }
        for(i32 i = 0; i < s.mem_names.size(); i++) {
            fout << "\t\t" << s.name << "_t._struct.member_offsets[" << i << "] = offsetof("
                 << s.name << ", " << s.mem_names[i] << ");" << endl;
        }
        fout << "\t\t" << s.name << "_t._struct.member_count = " << s.mem_types.size() << ";" << endl
             << "\t\t" << s.name << "_t.name = string_literal(\"" << s.name << "\");" << endl;
        fout << "\t\t" << s.name << "_t.hash = (u64)typeid(" << s.name << ").hash_code();" << endl
             << "\t\tmap_insert(&type_table, " << s.name << "_t.hash, " << s.name << "_t, false);" << endl
             << "\t}" << endl;
    }
}

void struct_out_template(const string& name, const string& type) {

    auto entry = _structs.find(name);
    _struct s = entry->second;

    string nametype = s.name + "<" + type + ">";

    fout << endl
         << "\t{" << endl
         << "\t\t// " << s.name << endl
         << "\t\t_type_info " << s.name << "_t;" << endl
         << "\t\t" << s.name << "_t.type_type = Type::_struct;" << endl
         << "\t\t" << s.name << "_t.size = sizeof(" << nametype << ");" << endl;

    for(i32 i = 0; i < s.mem_types.size(); i++) {
        string mtype = s.mem_types[i];
        if(mtype == "T") mtype = type;
        fout << "\t\t" << s.name << "_t._struct.member_types[" << i << "] = TYPEINFO("
             << mtype << ");" << endl;
    }
    for(i32 i = 0; i < s.mem_names.size(); i++) {
        fout << "\t\t" << s.name << "_t._struct.member_names[" << i << "] = string_literal(\""
             << s.mem_names[i] << "\");" << endl;
    }
    for(i32 i = 0; i < s.mem_names.size(); i++) {
        fout << "\t\t" << s.name << "_t._struct.member_offsets[" << i << "] = offsetof("
             << nametype << ", " << s.mem_names[i] << ");" << endl;
    }
    fout << "\t\t" << s.name << "_t._struct.member_count = " << s.mem_types.size() << ";" << endl
         << "\t\t" << s.name << "_t.name = string_literal(\"" << nametype << "\");" << endl;
    fout << "\t\t" << s.name << "_t.hash = (u64)typeid(" << nametype << ").hash_code();" << endl
         << "\t\tmap_insert(&type_table, " << s.name << "_t.hash, " << s.name << "_t, false);" << endl
         << "\t}" << endl;
}

void load(string file) {

	ifstream fin(file, ios::binary | ios::ate);
	streamsize fsize = fin.tellg();
	fin.seekg(0, ios::beg);

	if(!fin.good()) {
		printf("failed to open file %s\n", file.c_str());
		return;
	}

	vector<char> data((u32)fsize);
	fin.read(data.data(), fsize);

	Cpp_Token_Array tokens = cpp_make_token_array(2048);
	cpp_lex_file(data.data(), (u32)fsize, &tokens);

	for(i32 i = 0; i < tokens.count; i++) {
		Cpp_Token token = tokens.tokens[i];

		if (token.type == CPP_PP_INCLUDE_FILE) {
			char* place = data.data() + token.start;
			string incl(place, token.size);
			if (incl[0] == '\"') {

				all_tokens.pop_back();
				incl = path + incl.substr(1, incl.size() - 2);

				if (files.find(incl) == files.end() && incl != "meta_types.h") {
					files.insert(incl);
					load(incl);
				}

			}
			else {
				lib_includes.push_back(all_tokens.back());
				all_tokens.pop_back();
				lib_includes.push_back(_token(token, data.data()));
			}
		} else if (token.type == CPP_TOKEN_COMMENT) {

			_token t(token, data.data());
			t.str = t.str.substr(0, t.str.size() - 1);
			all_tokens.push_back(t);

		} else {

			all_tokens.push_back(_token(token, data.data()));
		}
   	}

   	cpp_free_token_array(tokens);
}

i32 main(i32 argc, char** argv) {
    if (argc < 2){
        printf("usage: %s <filename>\n", argv[0]);
        return 1;
    }

    fout.open("w:/src/meta_types.h");

    if(!fout.good()) {
    	printf("failed to create file meta_types.h\n");
    	return 1;
    }

    string first(argv[1]); 
    path = first.substr(0, first.find_last_of("\\/") + 1);
    load(first);

    for(i32 i = 0; i < lib_includes.size(); i += 2) {
    	cout << lib_includes[i].str << " " << lib_includes[i+1].str << endl;
    }
    cout << endl;

    fout << endl << "void make_meta_types() {" << endl;
    for(i32 i = 0; i < all_tokens.size(); i++) {
    	_token& t = all_tokens[i];

        bool templated = false;
    	if(t.str == "template") {
            if(all_tokens[i+1].str == "<") {
        		do {
        			t = all_tokens[++i];
    			} while (t.str != "struct" && t.str != "union" && t.str != ";");
                templated = true;
            } else {
                t = all_tokens[++i];
                string name = t.str;
                t = all_tokens[++i];
                if(t.str == "<") {
                    t = all_tokens[++i];
                    string type = t.str;
                    t = all_tokens[++i];
                    t = all_tokens[++i];
                    struct_out_template(name, type);
                } else {
                    do {
                        t = all_tokens[++i];
                    } while (t.str != "struct" && t.str != "union" && t.str != ";");
                }
            }
    	}
        bool skip = false;
    	if((t.str == "struct" || t.str == "union") && all_tokens[i + 1].str != "{") {
    		if(all_tokens[i + 2].str == ";") continue;
            
            _struct s;
			string name = all_tokens[i + 1].str;
			s.name = name;
            s.templated = templated;

            while(all_tokens[i].str != "{") i++;

    		u32 depth = 1;
    		for(i32 j = i + 1;; j++, i++) {
    			string type, name;

                while(all_tokens[j].token.type == CPP_TOKEN_COMMENT) {
                    if(all_tokens[j++].str.find("@NORTTI") != string::npos) {
                        cout << all_tokens[j].str << endl;
                        skip = true;
                        break;
                    }
                }

    			if(depth == 0) {
    				break;
    			}
    			type = all_tokens[j].str;

				if(type == "}") {
    				depth--;
    				j++;
    				continue;
    			}

    			if(all_tokens[j + 1].str == "*") {
    				type += all_tokens[++j].str;	
    			}
				name = all_tokens[++j].str; // skip template member
                if(name == "<") {
                    while(all_tokens[++j].str != ";");
                    while(all_tokens[j+1].token.type == CPP_TOKEN_COMMENT) j++;
                    continue;
                }
				if(name == "(" || all_tokens[j + 1].str == "(" || type == "~") {
					if(all_tokens[j + 1].str == "*") { // skip function pointer
						while(all_tokens[++j].str != ";");
						while(all_tokens[j+1].token.type == CPP_TOKEN_COMMENT) j++;
						continue;
					}
					j++;
					u32 mem_depth = 1;
					while(all_tokens[j].str != "{") j++;
					for(j++;mem_depth;j++) {
						if(all_tokens[j].str == "{") mem_depth++;
						if(all_tokens[j].str == "}") mem_depth--;
					}
					if(all_tokens[j].str == ";") {
						j++;
					}
					j--;
					continue;
				}
				if(all_tokens[++j].str == "=") {
					while(all_tokens[++j].str != ";");
					while(all_tokens[j+1].token.type == CPP_TOKEN_COMMENT) j++;
				}

    			if(type == "struct" || type == "union") {
    				depth++;
    				j--;
    				continue;
    			}

    			if(all_tokens[j].str == "[") { // skip arrays
    				while(all_tokens[++j].str != ";");
                    while(all_tokens[j+1].token.type == CPP_TOKEN_COMMENT) j++;
    			}
                if(all_tokens[j].str == "=") {
                    while(all_tokens[++j].str != ";");
                    while(all_tokens[j+1].token.type == CPP_TOKEN_COMMENT) j++;
                }

    			s.mem_types.push_back(type);
    			s.mem_names.push_back(name);
    		}

            if(!skip) {
                struct_out(s);
                _structs.insert({s.name, s});
            }
    	}

    	if(t.str == "enum") {

    		string ename = all_tokens[i+2].str;
    		fout << endl
    			 << "\t{" << endl
    			 << "\t\t// " << ename << endl
    			 << "\t\t_type_info " << ename << "_t;" << endl
    			 << "\t\t" << ename << "_t.type_type = Type::_enum;" << endl
    			 << "\t\t" << ename << "_t.size = sizeof(" << ename << ");" << endl
                 << "\t\t" << ename << "_t.name = string_literal(\"" << ename << "\");" << endl
    			 << "\t\tu64 hash = (u64)typeid(" << ename << ").hash_code();" << endl
    			 << "\t\tmap_insert(&type_table, hash, " << ename << "_t, false);" << endl
    			 << "\t}" << endl;
    	}
    }
    fout << endl << "}";

    fout << endl;
    fout.close();

    return 0;
}


