
#include <clang-c/Index.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <map>
#include <string>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <cctype>
#include <locale>

#include "basic_types.h"

using namespace std;

struct struct_def {
	string name;
	CXCursor this_;
	vector<CXCursor> members;

	bool is_template 		= false;
	bool is_explicit_inst 	= false;
	vector<string> template_type_params;

	vector<function<void(ofstream&)>> dependancies;
};
vector<pair<struct_def, map<string, CXType>>> done;
vector<struct_def> structs;
struct_def current_struct_def;
vector<CXType> current_instantiation;

ostream& operator<<(ostream&, const CXString&);
bool is_ds_decl(CXCursorKind);
bool is_fwd_decl(CXCursor);
CXChildVisitResult do_parse(CXCursor);
void output_pre(ofstream&);
void output_post(ofstream&);
void output_struct(ofstream&, const struct_def&);
void output_template_struct(ofstream&, const struct_def&, const map<string, CXType>&);
map<string, CXType> make_translation(const struct_def&, const vector<CXType>&);
bool operator==(const CXType& one, const CXType& two) {
	return memcmp(&one, &two, sizeof(one)) == 0;
}

void trim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

template<typename Out>
void split(const string &s, char delim, Out result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        *(result++) = item;
    }
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

ostream& operator<<(ostream& stream, const CXString& str) {
	stream << clang_getCString(str);
	clang_disposeString(str);
	return stream;
}

bool is_ds_decl(CXCursorKind k) {
	return k == CXCursor_StructDecl		||
		   k == CXCursor_EnumDecl		||
		   k == CXCursor_UnionDecl		||
		   k == CXCursor_ClassTemplate	||
		   k == CXCursor_ClassTemplatePartialSpecialization;
}

bool is_fwd_decl(CXCursor cursor) {
	return clang_isCursorDefinition(cursor) == 0;
}

CXChildVisitResult parse_struct_or_union(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(c.kind == CXCursor_FieldDecl) {
		
		current_struct_def.members.push_back(c);

		auto type = clang_getCursorType(c);
		i32 num_args = clang_Type_getNumTemplateArguments(type);
		if(num_args != -1 && !current_struct_def.is_template) {
			auto cx_instname = clang_getTypeSpelling(type);
			string instname(clang_getCString(cx_instname));
			clang_disposeString(cx_instname);

			instname = instname.substr(0, instname.find_first_of("<"));
			auto entry = find_if(structs.begin(), structs.end(), [instname](struct_def& def) -> bool { return instname == def.name; });
			if(entry != structs.end()) {
				vector<CXType> instantiation;

				for(u32 i = 0; i < (u32)num_args; i++) {
					instantiation.push_back(clang_Type_getTemplateArgumentAsType(type, i));
				}

				auto def = *entry;
				current_struct_def.dependancies.push_back([instantiation, def](ofstream& fout) -> void { output_template_struct(fout, def, make_translation(def, instantiation)); });
			}
		}

	} else if(clang_Cursor_isAnonymous(c)) {
		
		clang_visitChildren(c, parse_struct_or_union, nullptr);

	} else if(c.kind == CXCursor_TypeRef) {

		current_struct_def.is_explicit_inst = true;
		current_instantiation.push_back(clang_getCursorType(c));

	} else if(c.kind == CXCursor_TemplateTypeParameter) {

		current_struct_def.is_template = true;
		auto cx_type_arg = clang_getCursorSpelling(c);
		string type_arg(clang_getCString(cx_type_arg));
		clang_disposeString(cx_type_arg);

		current_struct_def.template_type_params.push_back(type_arg);

	} else {

		// cout << c.kind << endl;
	}
	return CXChildVisit_Continue;
}

CXChildVisitResult do_parse(CXCursor c) {
	if(is_fwd_decl(c)) return CXChildVisit_Continue;

	switch(c.kind) {
	case CXCursor_ClassTemplate:
	case CXCursor_UnionDecl:
	case CXCursor_StructDecl: {
		current_struct_def = struct_def();
		current_struct_def.this_ = c;

		auto cx_name = clang_getCursorSpelling(c);
		string name(clang_getCString(cx_name));
		clang_disposeString(cx_name);

		current_struct_def.name = name;

		clang_visitChildren(c, parse_struct_or_union, nullptr);

		if(current_struct_def.is_explicit_inst) {
			
			auto _current_instantiation = move(current_instantiation);
			auto entry = find_if(structs.begin(), structs.end(), [name](struct_def& def) -> bool { return name == def.name; });
			if(entry != structs.end()) {
				struct_def def = *entry;
				entry->dependancies.push_back([_current_instantiation, def](ofstream& fout) -> void { output_template_struct(fout, def, make_translation(def, _current_instantiation)); });
			}
			current_instantiation.clear();
		} else {
			structs.push_back(current_struct_def);
		}
	} break;
	case CXCursor_EnumDecl: {
		// TODO(max)
	} break;
	case CXCursor_ClassTemplatePartialSpecialization: {
		// could do this but we only use it once for _get_type_info
	} break;
	case CXCursor_VarDecl:
	case CXCursor_ParmDecl: {
		// test if needs to add an instantiation
	} break;
	}
	return CXChildVisit_Continue;
}

void output_pre(ofstream& fout) {
	fout << endl
		 << "void make_meta_types() {" << endl
		 << endl;
}

void output_post(ofstream& fout) {
	fout << "}" << endl;
}

void output_struct(ofstream& fout, const struct_def& s) {

	auto cx_name = clang_getCursorSpelling(s.this_);
	string name(clang_getCString(cx_name));
	clang_disposeString(cx_name);

	for(auto& f : s.dependancies) {
		f(fout);
	}

	if(s.is_template) return;

	fout << "\t// " << name << endl;
}

map<string, CXType> make_translation(const struct_def& s, const vector<CXType>& instantiation) {
	map<string, CXType> translation;
	for(auto type_param : s.template_type_params) {
		trim(type_param);
		u32 index = 0;
		for(;; index++) {
			if(s.template_type_params[index] == type_param) {
				break;
			}
		}
		translation.insert({type_param, instantiation[index]});
	}
	return translation;
}

void output_template_struct(ofstream& fout, const struct_def& s, const map<string, CXType>& translation) {
	
	if(find_if(done.begin(), done.end(), [&](const auto& val) -> bool {return val.first.name == s.name && val.second == translation;}) != done.end()) return;

	auto cx_name = clang_getCursorSpelling(s.this_);
	string name(clang_getCString(cx_name));
	clang_disposeString(cx_name);

	fout << endl;
	if(s.is_template) {

		for(auto& t : s.template_type_params) {

			auto entry = translation.find(t);
			if(entry != translation.end()) {
				fout << "#pragma push_macro(\"" << entry->first << "\")" << endl;
				fout << "#define " << entry->first << " " << clang_getTypeSpelling(entry->second) << endl;
			}
		}
		fout << "{" << endl;

		for(auto& member : s.members) {
			auto mem_type = clang_getCursorType(member);
			i32 mem_templ_args = clang_Type_getNumTemplateArguments(mem_type);
			if(mem_templ_args != -1) {
				auto cx_mem_name = clang_getTypeSpelling(mem_type);
				string mem_name(clang_getCString(cx_mem_name));
				clang_disposeString(cx_mem_name);

				mem_name = mem_name.substr(0, mem_name.find_first_of("<"));

				cout << mem_name << endl;
				auto entry = find_if(structs.begin(), structs.end(), [mem_name](struct_def& def) -> bool { return mem_name == def.name; });

				map<string, CXType> mem_translation;
				u32 idx = 0;
				for(auto& mem_param : entry->template_type_params) {
					
					auto mem_arg_type = clang_Type_getTemplateArgumentAsType(mem_type, idx);
					auto mem_entry = translation.find(mem_param);
					if(mem_entry != translation.end()) {
						mem_arg_type = mem_entry->second;
					}

					mem_translation.insert({mem_param, mem_arg_type});
					idx++;
				}
				output_template_struct(fout, *entry, mem_translation);
			}
		}

		fout << "\t" << name << "<";
		for(u32 idx = 0; idx < s.template_type_params.size(); idx++) {
			
			fout << s.template_type_params[idx];
			if(idx != s.template_type_params.size() - 1) {
				fout << ",";
			}
		}
		fout << "> owo;" << endl;

		fout << "}" << endl;
		for(auto& t : s.template_type_params) {

			auto entry = translation.find(t);
			if(entry != translation.end()) {
				fout << "#pragma pop_macro(\"" << entry->first << "\")" << endl;
			}
		}

		done.push_back({s, translation});
	}
	fout << endl;
}

i32 main(i32 argc, char** argv) {

	if(argc < 2) {
		cout << "Incorrect usage." << endl;
		return -1;
	}

	auto index = clang_createIndex(0, 0);
	auto unit = clang_parseTranslationUnit(index, argv[1], nullptr, 0, nullptr, 0, CXTranslationUnit_SkipFunctionBodies);

	if (unit == nullptr) {
		cout << "Unable to parse translation unit." << endl;
		return -1;
	}

	auto cursor = clang_getTranslationUnitCursor(unit);

	clang_visitChildren(cursor,
	[](CXCursor c, CXCursor parent, CXClientData client_data) {

		if(clang_Location_isInSystemHeader(clang_getCursorLocation(c))) {
			return CXChildVisit_Continue;
		}

		return do_parse(c);
	}, nullptr);

	ofstream fout("meta_types.h");
	output_pre(fout);
	for(auto& s : structs) {
		output_struct(fout, s);
	}
	output_post(fout);
	fout.close();

	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);

	cout << "Done!" << endl;

	return 0;
}
