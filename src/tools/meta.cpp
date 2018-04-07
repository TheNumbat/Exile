
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
#include <string>

#include "../engine/util/basic_types.h"

using namespace std;

struct enum_def {
	string name;
	CXCursor this_;
	CXType underlying;
	struct member {
		string name;
		i64 value;
	};
	vector<member> members;
};
struct struct_def {
	string name;
	CXCursor this_;
	vector<pair<bool,CXCursor>> members;

	bool is_template 		= false;
	bool is_explicit_inst 	= false;
	bool noreflect		 	= false;
	vector<string> template_type_params;

	vector<function<void(ofstream&)>> templ_deps;
	vector<function<void(ofstream&)>> arr_deps;
	vector<function<void(ofstream&)>> ptr_deps;
};

// NOTE(max): I realized that instead of the whole done system I could've just used
// 			  map_insert_if_unique, which would have been a _lot_ simpler. However,
//			  this works robustly (so far...), so I won't change it. I will use it
// 			  for array meta gen, though.

// using dictionaries would be faster, but we need to preserve ordering (could sort based on monotonic ID but w/e)
// also, this is already fast enough, it's a small portion of the full build time
vector<pair<struct_def, map<string, CXType>>> done;
vector<struct_def> structs;
struct_def current_struct_def;
vector<enum_def> enums;
enum_def current_enum_def;
vector<function<void(ofstream&)>> var_parm_deps;

vector<CXType> current_instantiation;

// string utility, because the STL is so fully featured
void trim(string &s);
template<typename Out> void split(const string &s, char delim, Out result);
vector<string> split(const string &s, char delim);
ostream& operator<<(ostream& stream, const CXString& str);
string str(CXString cx_str);

bool operator==(const CXType& one, const CXType& two);
bool is_fwd_decl(CXCursor c);

CXChildVisitResult parse_enum(CXCursor c, CXCursor parent, CXClientData client_data);
CXChildVisitResult parse_struct_or_union(CXCursor c, CXCursor parent, CXClientData client_data);
CXChildVisitResult do_parse(CXCursor c);
void try_add_template_dep(CXType type);

void output_pre(ofstream& fout);
void output_post(ofstream& fout);
void output_func(ofstream& fout, CXType type, CXCursor func);
void output_array(ofstream& fout, CXType type);
void output_enum(ofstream& fout, const enum_def& e);
void output_struct(ofstream& fout, const struct_def& s);
void output_template_struct(ofstream& fout, const struct_def& s, const map<string, CXType>& trans);
map<string, CXType> make_translation(const struct_def& s, const vector<CXType>& inst);
void print_templ_struct(ofstream& fout, const struct_def& s, const map<string, CXType>& trans);


bool operator==(const CXType& one, const CXType& two) {
	return memcmp(&one, &two, sizeof(one)) == 0;
}

void trim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
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

void remove_all_whitespace(string& s) {
	for(u32 i = 0; i < s.size(); i++) {
		if(isspace(s[i])) {
			s.erase(i, 1);
			i--;
		}
	}
}

ostream& operator<<(ostream& stream, const CXString& str) {
	stream << clang_getCString(str);
	clang_disposeString(str);
	return stream;
}

string str(CXString cx_str) {
	string str(clang_getCString(cx_str));
	clang_disposeString(cx_str);
	return str;
}

bool is_fwd_decl(CXCursor c) {
	return clang_isCursorDefinition(c) == 0;
}

CXChildVisitResult parse_enum(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(c.kind == CXCursor_EnumConstantDecl) {

		enum_def::member member;
		member.value = clang_getEnumConstantDeclValue(c);
		member.name  = str(clang_getCursorSpelling(c));

		current_enum_def.members.push_back(member);

	} else {
		
		// cout << c.kind << " " << clang_getCursorSpelling(c) << endl;
	}

	return CXChildVisit_Continue;
}

void try_add_template_dep(CXType type) {
	i32 num_args = clang_Type_getNumTemplateArguments(type);
	if(num_args != -1 && !current_struct_def.is_template) {
		auto instname = str(clang_getTypeSpelling(type));

		instname = instname.substr(0, instname.find_first_of("<"));
		auto entry = find_if(structs.begin(), structs.end(), [instname](struct_def& def) -> bool { return instname == def.name; });
		if(entry != structs.end()) {
			vector<CXType> instantiation;

			for(u32 i = 0; i < (u32)num_args; i++) {
				instantiation.push_back(clang_Type_getTemplateArgumentAsType(type, i));
			}

			auto def = *entry;
			current_struct_def.templ_deps.push_back([instantiation, def](ofstream& fout) -> void { output_template_struct(fout, def, make_translation(def, instantiation)); });
		}
	}
}

CXChildVisitResult attr_visit(CXCursor cursor, CXCursor parent, CXClientData data) {
    if (clang_isAttribute(cursor.kind)) {
        *(CXCursor*)data = cursor;
        return CXChildVisit_Break;
    }
    return CXChildVisit_Continue;
}

CXCursor first_attr(const CXCursor& c) {
    CXCursor attr;
    unsigned visit_result = clang_visitChildren(c, attr_visit, &attr);
    if (!visit_result) // attribute not found
        attr = clang_getNullCursor();
    return attr;
}

CXChildVisitResult parse_struct_or_union(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(c.kind == CXCursor_FieldDecl) {
		
		bool circular = false;
		CXCursor attr = first_attr(c);
		if(!clang_Cursor_isNull(attr)) {
			auto annotation = str(clang_getCursorSpelling(attr));
			if(annotation == "circular") {
				circular = true;
			}
		}

		auto type = clang_getCursorType(c);
		if(type.kind == CXType_ConstantArray) {
			current_struct_def.arr_deps.push_back([type](ofstream& fout) -> void {output_array(fout, type);});
		}

		if(type.kind == CXType_Pointer) {
			auto ptr_type = clang_getPointeeType(type);
			if(clang_getNumArgTypes(ptr_type) != -1) {
				current_struct_def.ptr_deps.push_back([ptr_type, c](ofstream& fout) -> void {output_func(fout, ptr_type, c);});						
			} else {
			
				try_add_template_dep(ptr_type);
			}
		}

		current_struct_def.members.push_back({circular,c});
		try_add_template_dep(type);

	} else if(clang_Cursor_isAnonymous(c)) {
		
		struct_def temp = current_struct_def;
		current_struct_def = struct_def();
		
		clang_visitChildren(c, parse_struct_or_union, nullptr);

		temp.members.insert(temp.members.end(), current_struct_def.members.begin(), current_struct_def.members.end());

		current_struct_def = temp;
		
	} else if(c.kind == CXCursor_TypeRef) {

		current_struct_def.is_explicit_inst = true;
		current_instantiation.push_back(clang_getCursorType(c));

	} else if(c.kind == CXCursor_TemplateTypeParameter) {

		current_struct_def.is_template = true;
		auto type_arg = str(clang_getCursorSpelling(c));

		current_struct_def.template_type_params.push_back(type_arg);

	} else if(c.kind == CXCursor_AnnotateAttr) {

		auto annotation = str(clang_getCursorSpelling(c));
		if(annotation == "noreflect") {
			current_struct_def.noreflect = true;
		}

	} else {

		// cout << c.kind << " " << clang_getCursorSpelling(c) << " " << clang_getTypeSpelling(clang_getCursorType(c)) << endl;
	}
	return CXChildVisit_Continue;
}

CXChildVisitResult do_parse(CXCursor c) {

	if(is_fwd_decl(c)) return CXChildVisit_Continue;

	switch(c.kind) {
	case CXCursor_ClassTemplatePartialSpecialization: {
		// ignore this, it's only _get_type_info<T*> and it'd mess with stuff
		return CXChildVisit_Continue;
	} break; 
	case CXCursor_ClassTemplate:
	case CXCursor_UnionDecl:
	case CXCursor_StructDecl: {
		current_struct_def = struct_def();
		current_struct_def.this_ = c;

		auto name = str(clang_getCursorSpelling(c));
		if(!name.size()) return CXChildVisit_Continue;
		current_struct_def.name = name;

		clang_visitChildren(c, parse_struct_or_union, nullptr);

		if(current_struct_def.is_explicit_inst) {
			
			auto _current_instantiation = move(current_instantiation);
			auto entry = find_if(structs.begin(), structs.end(), [name](struct_def& def) -> bool { return name == def.name; });
			if(entry != structs.end()) {
				struct_def def = *entry;
				entry->templ_deps.push_back([_current_instantiation, def](ofstream& fout) -> void { output_template_struct(fout, def, make_translation(def, _current_instantiation)); });
			}
			current_instantiation.clear();
		} else if(!current_struct_def.noreflect) {
			structs.push_back(current_struct_def);
		}

		return CXChildVisit_Continue;
	} break;
	case CXCursor_EnumDecl: {
		
		current_enum_def = enum_def();
		current_enum_def.this_ = c;
		current_enum_def.underlying = clang_getEnumDeclIntegerType(c);

		auto name = str(clang_getCursorSpelling(c));
		if(!name.size()) return CXChildVisit_Continue;
		current_enum_def.name = name;

		clang_visitChildren(c, parse_enum, nullptr);

		// sort values for binary search - stable so we can choose the first/last name for the value when printing
		stable_sort(current_enum_def.members.begin(), current_enum_def.members.end(), [](auto& l, auto& r) -> bool { return l.value < r.value; });
		enums.push_back(current_enum_def);

		return CXChildVisit_Continue;
	} break;
	case CXCursor_VarDecl:
	case CXCursor_ParmDecl: {
		auto type = clang_getCursorType(c);
		if(clang_Type_getNumTemplateArguments(type) != -1) {
			auto name = str(clang_getTypeSpelling(type));
			auto idx = name.find_first_of("<");

			if(idx != string::npos) {
				name = name.substr(0, idx);

				vector<CXType> instantiation;
				for(i32 i = 0; i < clang_Type_getNumTemplateArguments(type); i++) {
					instantiation.push_back(clang_Type_getTemplateArgumentAsType(type, i));
				}

				auto entry = find_if(structs.begin(), structs.end(), [name](struct_def& def) -> bool { return name == def.name; });
				if(entry != structs.end()) {
					struct_def def = *entry;
					auto translation = make_translation(def, instantiation);

					bool fully_specified = true;
					for(auto& e : translation) {
						if(clang_Type_getSizeOf(e.second) == CXTypeLayoutError_Dependent) {
							fully_specified = false;
						}
					}
					if(fully_specified)
						var_parm_deps.push_back([translation, def](ofstream& fout) -> void {output_template_struct(fout, def, translation);});
				}
			}
		}
		return CXChildVisit_Continue;
	} break;
	default: {
		return CXChildVisit_Recurse;
	} break;
	}
}

void output_pre(ofstream& fout) {
	fout << endl << "#define STRING2(...) #__VA_ARGS__##_" << endl
		 << "#define STRING(...) STRING2(__VA_ARGS__)" << endl
		 << "void make_meta_info() { PROF" << endl << endl;
}

void output_post(ofstream& fout) {
	fout << "}" << endl;
}

void output_func(ofstream& fout, CXType type, CXCursor func) {

	auto ret = str(clang_getTypeSpelling(clang_getResultType(type)));
	auto signature = str(clang_getTypeSpelling(type));
	auto num_args = (u32)clang_getNumArgTypes(type);
	auto name = str(clang_getCursorSpelling(func));

	fout << "\t[]() -> void {" << endl
		 << "\t\t_type_info this_type_info;" << endl
		 << "\t\tthis_type_info.type_type = Type::_func;" << endl
		 << "\t\tthis_type_info.size = sizeof(void(*)());" << endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << signature << ").hash_code();" << endl
		 << "\t\tthis_type_info.name = \"" << name << "\"_;" << endl
		 << "\t\tthis_type_info._func.signature = STRING(" << signature << ");" << endl
		 << "\t\tthis_type_info._func.return_type = TYPEINFO(" << ret << ") ? TYPEINFO(" << ret << ")->hash : 0;" << endl
		 << "\t\tthis_type_info._func.param_count = " << num_args << ";" << endl;

	for(u32 i = 0; i < num_args; i++) {
		auto arg = str(clang_getTypeSpelling(clang_getArgType(type, i)));

		fout << "\t\tthis_type_info._func.param_types[" << i << "] = TYPEINFO(" << arg << ") ? TYPEINFO(" << arg << ")->hash : 0;" << endl;
	}

	fout << "\t\ttype_table.insert_if_unique(this_type_info.hash, this_type_info, false);" << endl
		 << "\t}();" << endl << endl;
}

void output_array(ofstream& fout, CXType type) {

	auto ele_type = clang_getArrayElementType(type);
	if(ele_type.kind == CXType_ConstantArray) {
		output_array(fout, ele_type);
	}

	auto name = str(clang_getTypeSpelling(type));
	auto base = str(clang_getTypeSpelling(ele_type));

	fout << "\t[]() -> void {" << endl
		 << "\t\t_type_info this_type_info;" << endl
		 << "\t\tthis_type_info.type_type = Type::_array;" << endl
		 << "\t\tthis_type_info.size = sizeof(" << name << ");" << endl
		 << "\t\tthis_type_info.name = STRING(" << name << ");" << endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << name << ").hash_code();" << endl
		 << "\t\tthis_type_info._array.of = TYPEINFO(" << base << ") ? TYPEINFO(" << base << ")->hash : 0;" << endl
		 << "\t\tthis_type_info._array.length = " << clang_getNumElements(type) << ";" << endl
		 << "\t\ttype_table.insert_if_unique(this_type_info.hash, this_type_info, false);" << endl
		 << "\t}();" << endl << endl;
}

void output_enum(ofstream& fout, const enum_def& e) {

	auto& name = e.name;
	auto type = str(clang_getTypeSpelling(e.underlying));

	if(e.members.size() > 256) {
		cout << "enum " << name << " has too many members!" << endl;
		return;
	}

	fout << "\t[]() -> void {" << endl
		 << "\t\t_type_info this_type_info;" << endl
		 << "\t\tthis_type_info.type_type = Type::_enum;" << endl
		 << "\t\tthis_type_info.size = sizeof(" << type << ");" << endl
		 << "\t\tthis_type_info.name = \"" << name << "\"_;" << endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << name << ").hash_code();" << endl
		 << "\t\tthis_type_info._enum.member_count = " << e.members.size() << ";" << endl
		 << "\t\tthis_type_info._enum.base_type = TYPEINFO(" << type << ") ? TYPEINFO(" << type << ")->hash : 0;" << endl;

	u32 idx = 0;
	for(auto& member : e.members) {
		fout << "\t\tthis_type_info._enum.member_names[" << idx << "] = \"" << member.name << "\"_;" << endl
			 << "\t\tthis_type_info._enum.member_values[" << idx << "] = " << member.value << ";" << endl;
		idx++;
	}

	fout << "\t\ttype_table.insert(this_type_info.hash, this_type_info, false);" << endl
		 << "\t}();" << endl << endl;
}

void output_struct(ofstream& fout, const struct_def& s) {

	auto& name = s.name;
	if(find_if(done.begin(), done.end(), [&](const auto& val) -> bool {return val.first.name == s.name;}) != done.end()) return;

	auto type = clang_getCursorType(s.this_);

	if(s.members.size() > 64) {
		cout << "struct " << name << " has too many members!" << endl;
		return;
	}

	for(auto& f : s.templ_deps) {
		f(fout);
	}

	if(s.is_template) return;

	for(auto& f : s.arr_deps) {
		f(fout);
	}
	for(auto& f : s.ptr_deps) {
		f(fout);
	}

	fout << "\t[]() -> void {" << endl;

	fout << "\t\t_type_info this_type_info;" << endl
		 << "\t\tthis_type_info.type_type = Type::_struct;" << endl
		 << "\t\tthis_type_info.size = sizeof(" << name << ");" << endl
		 << "\t\tthis_type_info.name = \"" << name << "\"_;" << endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << name << ").hash_code();" << endl
		 << "\t\tthis_type_info._struct.member_count = " << s.members.size() << ";" << endl;

	u32 idx = 0;
	for(auto& member : s.members) {
		auto mem_name = str(clang_getCursorSpelling(member.second));
		auto mem_type_name = str(clang_getTypeSpelling(clang_getCursorType(member.second)));

		fout << "\t\tthis_type_info._struct.member_types[" << idx << "] = TYPEINFO(" << mem_type_name << ") ? TYPEINFO(" << mem_type_name << ")->hash : 0;" << endl
			 << "\t\tthis_type_info._struct.member_names[" << idx << "] = \"" << mem_name << "\"_;" << endl
			 << "\t\tthis_type_info._struct.member_offsets[" << idx << "] = offsetof(" << name << "," << mem_name << ");" << endl
			 << "\t\tthis_type_info._struct.member_circular[" << idx << "] = " << member.first << ";" << endl;

		idx++;
	}

	fout << "\t\ttype_table.insert(this_type_info.hash, this_type_info, false);" << endl
		 << "\t}();" << endl << endl;

	done.push_back({s, {}});
}

void output_template_struct(ofstream& fout, const struct_def& s, const map<string, CXType>& translation) {
	
	if(find_if(done.begin(), done.end(), [&](const auto& val) -> bool {return val.first.name == s.name && val.second == translation;}) != done.end()) return;

	auto name = str(clang_getCursorSpelling(s.this_));

	if(s.is_template) {

		for(auto& t : s.template_type_params) {

			auto entry = translation.find(t);
			if(entry != translation.end()) {
				fout << "#pragma push_macro(\"" << entry->first << "\")" << endl;

				auto type_str = str(clang_getTypeSpelling(entry->second));
				remove_all_whitespace(type_str);

				fout << "#define " << entry->first << " " << type_str << endl;
			}
		}

		for(auto& f : s.arr_deps) {
			f(fout);
		}
		for(auto& f : s.ptr_deps) {
			f(fout);
		}

		for(auto& member : s.members) {
			auto mem_type = clang_getCursorType(member.second);
			i32 mem_templ_args = clang_Type_getNumTemplateArguments(mem_type);
			if(mem_templ_args != -1) {
				auto mem_name = str(clang_getTypeSpelling(mem_type));
				mem_name = mem_name.substr(0, mem_name.find_first_of("<"));

				auto entry = find_if(structs.begin(), structs.end(), [mem_name](struct_def& def) -> bool { return mem_name == def.name; });

				map<string, CXType> mem_translation;
				u32 idx = 0;
				for(auto& mem_param : entry->template_type_params) {
					
					auto mem_arg_type = clang_Type_getTemplateArgumentAsType(mem_type, idx);
					auto mem_entry = translation.find(mem_param);

					if(clang_Type_getNumTemplateArguments(mem_arg_type) != -1) {
						auto mem_arg_type_name = str(clang_getTypeSpelling(mem_arg_type));
						mem_arg_type_name = mem_arg_type_name.substr(0, mem_arg_type_name.find_first_of("<"));
						auto mem_arg_entry = find_if(structs.begin(), structs.end(), [mem_arg_type_name](struct_def& def) -> bool { return mem_arg_type_name == def.name; });
						output_template_struct(fout, *mem_arg_entry, translation);
					}

					if(mem_entry != translation.end()) {
						mem_arg_type = mem_entry->second;
					}

					mem_translation.insert({mem_param, mem_arg_type});
					idx++;
				}
				for(auto& trans : translation) {
					mem_translation.insert(trans);
				}
				output_template_struct(fout, *entry, mem_translation);
			}
		}

		print_templ_struct(fout, s, translation);

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

void print_templ_struct(ofstream& fout, const struct_def& s, const map<string, CXType>& translation) {

	auto& name = s.name;
	auto type = clang_getCursorType(s.this_);

	fout << "\t[]() -> void {" << endl;

	string qual_name = name + "<";
	for(u32 idx = 0; idx < s.template_type_params.size(); idx++) {
		
		qual_name += s.template_type_params[idx];
		if(idx != s.template_type_params.size() - 1) {
			qual_name += ",";
		}
	}
	qual_name += ">";

	fout << "\t\t_type_info this_type_info;" << endl
		 << "\t\tthis_type_info.type_type = Type::_struct;" << endl
		 << "\t\tthis_type_info.size = sizeof(" << qual_name << ");" << endl
		 << "\t\tthis_type_info.name = \"" << name << "\"_;" << endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << qual_name << ").hash_code();" << endl
		 << "\t\tthis_type_info._struct.member_count = " << s.members.size() << ";" << endl;

	fout << "#define __" << name << "__ " << qual_name << endl;
	u32 idx = 0;
	for(auto& member : s.members) {
		auto mem_name = str(clang_getCursorSpelling(member.second));
		auto mem_type_name = str(clang_getTypeSpelling(clang_getCursorType(member.second)));

		fout << "\t\tthis_type_info._struct.member_types[" << idx << "] = TYPEINFO(" << mem_type_name << ") ? TYPEINFO(" << mem_type_name << ")->hash : 0;" << endl
			 << "\t\tthis_type_info._struct.member_names[" << idx << "] = \"" << mem_name << "\"_;" << endl
			 << "\t\tthis_type_info._struct.member_offsets[" << idx << "] = offsetof(__" << name << "__, " << mem_name << ");" << endl
			 << "\t\tthis_type_info._struct.member_circular[" << idx << "] = " << member.first << ";" << endl;

		idx++;
	}
	fout << "#undef __" << name << "__" << endl;

	fout << "\t\ttype_table.insert(this_type_info.hash, this_type_info, false);" << endl
		 << "\t}();" << endl << endl;
}

i32 main(i32 argc, char** argv) {

	if(argc < 2) {
		cout << "Incorrect usage." << endl;
		return -1;
	}

	auto index = clang_createIndex(0, 0);
	auto unit = clang_parseTranslationUnit(index, argv[1], argv + 2, argc - 2, nullptr, 0, CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing);

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

		do_parse(c);
		return CXChildVisit_Recurse;
	}, nullptr);

	ofstream fout("meta_types.cpp");
	output_pre(fout);
	for(auto& e : enums) {
		output_enum(fout, e);
	}
	for(auto& s : structs) {
		output_struct(fout, s);
	}
	for(auto& f : var_parm_deps) {
		f(fout);
	}
	output_post(fout);
	fout.close();

	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);

	return 0;
}
