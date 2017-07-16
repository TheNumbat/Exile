
#include <clang-c/Index.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <map>
#include <string>
#include <algorithm>

#include "basic_types.h"

using namespace std;

struct struct_def {
	string name;
	CXCursor this_;
	vector<CXCursor> members;

	bool is_template 		= false;
	bool is_explicit_inst 	= false;

	vector<function<void(ofstream&)>> dependancies;
};
vector<pair<struct_def, vector<CXType>>> done;
vector<struct_def> structs;
struct_def current_struct_def;
vector<CXType> current_instantiation;

ostream& operator<<(ostream&, const CXString&);
bool is_ds_decl(CXCursorKind);
bool is_fwd_decl(CXCursor);
CXChildVisitResult parse_struct_or_union(CXCursor, CXCursor, CXClientData);
CXChildVisitResult do_parse(CXCursor);
void output_pre(ofstream&);
void output_post(ofstream&);
void output_struct(ofstream&, const struct_def&);
void output_template_struct(ofstream& fout, const struct_def& s, const vector<CXType>& instantiation);
bool operator==(const CXType& one, const CXType& two) {
	return memcmp(&one, &two, sizeof(one)) == 0;
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
				current_struct_def.dependancies.push_back([instantiation, def](ofstream& fout) -> void { output_template_struct(fout, def, instantiation); });
			}
		}

	} else if(clang_Cursor_isAnonymous(c)) {
		
		clang_visitChildren(c, parse_struct_or_union, nullptr);

	} else if(c.kind == CXCursor_TypeRef) {

		current_struct_def.is_explicit_inst = true;
		current_instantiation.push_back(clang_getCursorType(c));
	}
	return CXChildVisit_Continue;
}

CXChildVisitResult do_parse(CXCursor c) {
	if(is_fwd_decl(c)) return CXChildVisit_Continue;

	switch(c.kind) {
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
				entry->dependancies.push_back([_current_instantiation, def](ofstream& fout) -> void { output_template_struct(fout, def, _current_instantiation); });
			}
			current_instantiation.clear();
		} else {
			structs.push_back(current_struct_def);
		}
	} break;
	case CXCursor_EnumDecl: {
		// TODO(max)
	} break;
	case CXCursor_ClassTemplate: {
		current_struct_def = struct_def();
		current_struct_def.this_ = c;
		current_struct_def.is_template = true;
		
		auto cx_name = clang_getCursorSpelling(c);
		string name(clang_getCString(cx_name));
		clang_disposeString(cx_name);

		current_struct_def.name = name;

		clang_visitChildren(c, parse_struct_or_union, nullptr);

		structs.push_back(current_struct_def);
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

void output_template_struct(ofstream& fout, const struct_def& s, const vector<CXType>& _instantiation) {

	vector<CXType> instantiation(_instantiation);
	if(find_if(done.begin(), done.end(), [&](const auto& val) -> bool {return val.first.name == s.name && val.second == instantiation;}) != done.end()) return;

	auto cx_name = clang_getCursorSpelling(s.this_);
	string name(clang_getCString(cx_name));
	clang_disposeString(cx_name);

	if(s.is_template) {

		for(auto& c : s.members) {
			auto type = clang_getCursorType(c);
			auto num_args = clang_Type_getNumTemplateArguments(type);
			if(num_args != -1) {

				auto cx_instname = clang_getTypeSpelling(type);
				string instname(clang_getCString(cx_instname));
				clang_disposeString(cx_instname);

				instname = instname.substr(0, instname.find_first_of("<"));
				auto entry = find_if(structs.begin(), structs.end(), [instname](struct_def& def) -> bool { return instname == def.name; });

				// vector<CXType> member_instantiation;
				// for(u32 i = 0; i < (u32)num_args; i++) {
				// 	auto template_param_kind = clang_Cursor_getTemplateArgumentKind(c, i);

				// 	if(template_param_kind == CXTemplateArgumentKind_Type) {
						
				// 		member_instantiation.push_back(instantiation.front());
				// 		instantiation.erase(instantiation.begin());

				// 		cout << "WOOW" << endl;

				// 	} else {

				// 		auto template_param_type = clang_Cursor_getTemplateArgumentType(c, i);
				// 		auto cx_mem_instname = clang_getTypeSpelling(type);
				// 		string mem_instname(clang_getCString(cx_mem_instname));
				// 		clang_disposeString(cx_mem_instname);
						
				// 		cout << template_param_kind << endl;
				// 		auto mem_entry = find_if(structs.begin(), structs.end(), [mem_instname](struct_def& def) -> bool { return mem_instname == def.name; });
				// 		output_template_struct(fout, *mem_entry, member_instantiation);
				// 	}
				// }

				output_template_struct(fout, *entry, instantiation);
			}
		}

		fout << "\t// " << name << "<";
		for(auto& type : instantiation) {
			fout << clang_getTypeSpelling(type) << ",";
		}
		fout << ">" << endl;

		done.push_back({s, instantiation});
	}
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

	ofstream fout("meta_types_new.h");
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
