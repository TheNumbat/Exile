
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

	set<string> templ_types;
    set<vector<string>> instantiations;
    map<string, vector<string>> member_templs;
    bool templated = false, skip = false;
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

void struct_out_template(const string& name, vector<string> types) {

    auto entry = _structs.find(name);
    _struct& _s = entry->second;

    if(_s.skip) return;

    if(_s.instantiations.find(types) != _s.instantiations.end()) {
        return;
    }
    _s.instantiations.insert(types);
    _struct s = _s;

    string nametype = s.name + "<";
    for(i32 i = 0; i < types.size(); i++) {
        nametype += types[i];
        if(i != types.size() - 1) {
            nametype += ",";
        } 
    }
    nametype += ">";

    string __name__ = "__" + s.name + "__";
    fout << endl
         << "\t{" << endl
         << "#define " << __name__ << " " << nametype << endl
         << "\t\t// " << s.name << endl
         << "\t\t_type_info " << s.name << "_t;" << endl
         << "\t\t" << s.name << "_t.type_type = Type::_struct;" << endl
         << "\t\t" << s.name << "_t.size = sizeof(" << __name__ << ");" << endl;

    map<string, i32> templs;
    i32 i_templs = 0;
    for(i32 i = 0; i < s.mem_types.size(); i++) {
        string mtype = s.mem_types[i];
        bool ptr = false;
        if(mtype.back() == '*') {
            mtype.pop_back();
            ptr = true;
        }
        if (s.templ_types.find(mtype) != s.templ_types.end()) {
            if (templs.find(mtype) == templs.end()) {
                templs.insert({ mtype, i_templs });
                mtype = types[i_templs];
                ++i_templs;
            }
            else {
                mtype = types[templs.find(mtype)->second];
            }
        }
        if(ptr) mtype.push_back('*');

        string mname = s.mem_names[i];
        if(s.member_templs.find(mname) != s.member_templs.end()) {
            auto entry = s.member_templs.find(mname);
            // vector<string> inst = entry->second;
            // for(string& str : inst) {
            //     str = types[templs.find(str)->second];
            // }
            struct_out_template(mtype, types);
            mtype += '<';
            for(i32 i = 0; i < types.size(); i++) {
                mtype += types[i];
                if(i < types.size() - 1) {
                    mtype += ",";
                }
            }
            mtype += '>';
        }

        fout << "\t\t" << s.name << "_t._struct.member_types[" << i << "] = TYPEINFO("
             << mtype << ") ? TYPEINFO(" << mtype << ")->hash : 0;" << endl;

        fout << "\t\t" << s.name << "_t._struct.member_names[" << i << "] = string_literal(\""
             << mname << "\");" << endl;

        fout << "\t\t" << s.name << "_t._struct.member_offsets[" << i << "] = offsetof("
             << __name__ << ", " << mname << ");" << endl;
    }
    fout << "\t\t" << s.name << "_t._struct.member_count = " << s.mem_types.size() << ";" << endl
         << "\t\t" << s.name << "_t.name = string_literal(\"" << nametype << "\");" << endl;
    fout << "\t\t" << s.name << "_t.hash = (u64)typeid(" << __name__ << ").hash_code();" << endl
         << "\t\tmap_insert(&type_table, " << s.name << "_t.hash, " << s.name << "_t, false);" << endl
         << "#undef " << __name__ << endl
         << "\t}" << endl;
}

void struct_out(const _struct& s) {
    if(s.skip) return;
    if(!s.templated) {
        fout << endl
             << "\t{" << endl
             << "\t\t// " << s.name << endl
             << "\t\t_type_info " << s.name << "_t;" << endl
             << "\t\t" << s.name << "_t.type_type = Type::_struct;" << endl
             << "\t\t" << s.name << "_t.size = sizeof(" << s.name << ");" << endl;

        for(i32 i = 0; i < s.mem_types.size(); i++) {

            string type = s.mem_types[i];
            string name = s.mem_names[i];
			bool ptr = false;
			if (type.back() == '*') {
				type.pop_back();
				ptr = true;
			}
            if(s.member_templs.find(name) != s.member_templs.end()) {
                auto entry = s.member_templs.find(name);
                struct_out_template(type, entry->second);
                type += '<';
                for(i32 i = 0; i < entry->second.size(); i++) {
                    type += entry->second[i];
                    if(i < entry->second.size() - 1) {
                        type += ",";
                    }
                }
                type += '>';
            }
			if (ptr) type.push_back('*');

            fout << "\t\t" << s.name << "_t._struct.member_types[" << i << "] = TYPEINFO("
                 << type << ") ? TYPEINFO(" << type << ")->hash : 0;" << endl;

            fout << "\t\t" << s.name << "_t._struct.member_names[" << i << "] = string_literal(\""
                 << name << "\");" << endl;

            fout << "\t\t" << s.name << "_t._struct.member_offsets[" << i << "] = offsetof("
                 << s.name << ", " << name << ");" << endl;
        }
        fout << "\t\t" << s.name << "_t._struct.member_count = " << s.mem_types.size() << ";" << endl
             << "\t\t" << s.name << "_t.name = string_literal(\"" << s.name << "\");" << endl;
        fout << "\t\t" << s.name << "_t.hash = (u64)typeid(" << s.name << ").hash_code();" << endl
             << "\t\tmap_insert(&type_table, " << s.name << "_t.hash, " << s.name << "_t, false);" << endl
             << "\t}" << endl;
    }
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

    fout.open("meta_types.h");

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
        set<string> maybe_templ_types;
        map<string, vector<string>> maybe_mem_templ;
    	if(t.str == "template") {
            if(all_tokens[i+1].str == "<") { // definition
				++i;
                do {
                    t = all_tokens[++i];
                    if(t.str == "typename") {
						t = all_tokens[++i];
                        maybe_templ_types.insert(t.str);
                    }
                    t = all_tokens[++i];
                } while(t.str == ",");
    			while (t.str != "struct" && t.str != "union" && t.str != ";") t = all_tokens[++i];
                templated = true;
            } else { // instantiation
                t = all_tokens[++i];
                string name = t.str;
                t = all_tokens[++i];
                if(t.str == "<") {
                    vector<string> types;
                    do {
                        t = all_tokens[++i];
                        types.push_back(t.str);
                        t = all_tokens[++i];
                    } while(t.str == ",");
                    t = all_tokens[++i];
                    struct_out_template(name, types);
                } else {
                    do {
                        t = all_tokens[++i];
                    } while (t.str != "struct" && t.str != "union" && t.str != ";");
                }
            }
    	}
    	if((t.str == "struct" || t.str == "union") && all_tokens[i + 1].str != "{") {
    		if(all_tokens[i + 2].str == ";") continue;
            
            _struct s;
			string name = all_tokens[i + 1].str;
			s.name = name;
            s.templated = templated;
            s.templ_types = maybe_templ_types;
            s.member_templs = maybe_mem_templ;

            while(all_tokens[i].str != "{") i++;

    		u32 depth = 1;
    		for(i32 j = i + 1;; j++, i++) {
    			string type, name;

                while(all_tokens[j].token.type == CPP_TOKEN_COMMENT) {
                    if(all_tokens[j++].str.find("@NORTTI") != string::npos) {
                        cout << all_tokens[j].str << endl;
                        s.skip = true;
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

				name = all_tokens[++j].str; // template member
                if(name == "<") {
                    vector<string> types;
                    do {
                        t = all_tokens[++j];
                        types.push_back(t.str);
                        t = all_tokens[++j];
                        if(t.str == "*") {
                            types.back() += t.str;
                            t = all_tokens[++j];
                        }
                    } while(t.str == ",");
                    if(all_tokens[j + 1].str == "*") {
                       type += all_tokens[++j].str;
                    }
                    name = all_tokens[++j].str;
                    s.member_templs.insert({name, types});
                    while(all_tokens[j+1].token.type == CPP_TOKEN_COMMENT) j++;
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

            struct_out(s);
            _structs.insert({s.name, s});
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
                 << "\t\t" << ename << "_t.hash = (u64)typeid(" << ename << ").hash_code();" << endl
                 << "\t\tmap_insert(&type_table, " << ename << "_t.hash, " << ename << "_t, false);" << endl
    			 << "\t}" << endl;
    	}
    }
    fout << endl << "}";

    fout << endl;
    fout.close();

    return 0;
}


