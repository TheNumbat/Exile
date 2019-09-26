
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include <clang-c/Index.h>
#include <clang-c/CXString.h>

std::ostream& operator<<(std::ostream& stream, const CXString& str) {
	const char* c_str = clang_getCString(str);
	if(c_str) {
		stream << c_str;
		clang_disposeString(str);
	}
	return stream;
}

std::string to_string(CXString cx_str) {
	std::string str(clang_getCString(cx_str));
	clang_disposeString(cx_str);
	return str;
}

struct type_data {
	bool done = false;
	
	bool in_progress_out = false;
	bool in_progress_grf = false;
	bool in_progress_unx = false;

	bool unexposed = false; // true -> unexposed, false -> maybe unexposed, check with function
	std::vector<CXCursor> references;

	type_data(CXType type) {
		unexposed = type.kind == CXType_Unexposed;
	}
};

namespace std {
	template <>
	struct hash<CXType> {
		std::size_t operator()(const CXType& k) const {
			return hash<string>()(to_string(clang_getTypeSpelling(k)));
		}
	};
}

bool operator==(const CXType& l, const CXType& r) {
	return clang_equalTypes(l, r);
}

std::unordered_map<CXType, type_data> types;
std::vector<CXType> back_patches;
std::unordered_set<CXType> done;
std::ofstream log_out, fout;

void print_data_type_help(CXType type, CXType parent);

void clear_in_progress_out() {
	for(auto& entry : types) {
		entry.second.in_progress_out = false;
	}
}

void clear_in_progress_grf() {
	for(auto& entry : types) {
		entry.second.in_progress_grf = false;
	}
}

void clear_in_progress_unx() {
	for(auto& entry : types) {
		entry.second.in_progress_unx = false;
	}
}

void ensure_in_graph(CXType type) {

	auto entry = types.find(type);
	if(entry == types.end()) {
		type_data data(type);
		types.insert({type, data});
	} 
}

CXChildVisitResult gather_fields(CXCursor c, CXCursor parent, CXClientData client_data) {

	std::vector<CXCursor>* data = (std::vector<CXCursor>*)client_data;

	switch(c.kind) {
	case CXCursor_FieldDecl: {
		data->push_back(c);
	} break;
	case CXCursor_UnionDecl:
	case CXCursor_ClassDecl:
	case CXCursor_StructDecl: {
		if(clang_Cursor_isAnonymous(c)) {
			clang_visitChildren(c, gather_fields, client_data);
		}
	} break;
	default: break;
	}

	return CXChildVisit_Continue;
}

void clang_Type_visitFields_NotBroken(CXType type, CXFieldVisitor visitor, CXClientData client_data) {

	std::vector<CXCursor> fields;

	CXCursor decl = clang_getTypeDeclaration(type);

	if(type.kind == CXType_Unexposed) {
		decl = clang_getSpecializedCursorTemplate(decl);
	}

	clang_visitChildren(decl, gather_fields, &fields);

	for(CXCursor f : fields) {
		visitor(f, client_data);
	}
}

void ensure_deps_in_graph_help(CXType type) {

	ensure_in_graph(type);
	auto entry = types.find(type);

	if(entry->second.in_progress_grf) return;
	entry->second.in_progress_grf = true;

	switch(type.kind) {
	case CXType_Pointer: ensure_deps_in_graph_help(clang_getPointeeType(type)); break;
	case CXType_ConstantArray: ensure_deps_in_graph_help(clang_getArrayElementType(type)); break;
	case CXType_Record: {
		clang_Type_visitFields_NotBroken(type, [](CXCursor c, CXClientData data) -> CXVisitorResult {
			ensure_deps_in_graph_help(clang_getCursorType(c));
			return CXVisit_Continue;
		}, nullptr);
	} break;
	case CXType_FunctionProto: {
		ensure_deps_in_graph_help(clang_getResultType(type));
		for(int i = 0; i < clang_getNumArgTypes(type); i++) {
			ensure_deps_in_graph_help(clang_getArgType(type, i));
		}
	} break;
	default: break;
	}
}

void ensure_deps_in_graph(CXType type) {

	ensure_deps_in_graph_help(type);
	clear_in_progress_grf();
}

void print_dep_type(CXType type, CXType parent) {
	
	auto entry = types.find(type);
	if(entry != types.end()) {
		print_data_type_help(entry->first, parent);
	} else {
		log_out << "WARN: dependency type " << clang_getTypeSpelling(type) << " for "
				<< clang_getTypeSpelling(parent) <<  " not found in graph!!" << std::endl;
	}
}

CXChildVisitResult traversal(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(clang_Location_isInSystemHeader(clang_getCursorLocation(c)) || 
	   clang_isPreprocessing(c.kind) || clang_isUnexposed(c.kind)  || 
	   clang_isInvalid(c.kind)) {

		return CXChildVisit_Continue;
	}

	if(c.kind == CXCursor_VarDecl || c.kind == CXCursor_ParmDecl ||
	   c.kind == CXCursor_CStyleCastExpr || c.kind == CXCursor_MemberRefExpr) {

		CXType type = clang_getCursorType(c);
		type_data data(type);
		data.references.push_back(c);
		types.insert({type, data});
	}

	if(c.kind == CXCursor_StructDecl || c.kind == CXCursor_UnionDecl || 
	   c.kind == CXCursor_ClassDecl) {

		if(clang_isCursorDefinition(c)) {
			
			CXType type = clang_getCursorType(c);
			type_data data(type);
			data.references.push_back(c);
			types.insert({type, data});

			log_out << "traversal type: " << clang_getTypeSpelling(type) << std::endl;
		}
	}

	return CXChildVisit_Recurse;
}

bool type_is_unexposed_help(CXType type) {

	ensure_deps_in_graph(type);

	auto entry = types.find(type);
	if(entry->second.unexposed) return true;

	if(entry->second.in_progress_unx) return entry->second.unexposed;
	entry->second.in_progress_unx = true;

	switch(type.kind) {
	case CXType_Pointer: {
		entry->second.unexposed = type_is_unexposed_help(clang_getPointeeType(type)); 
	} break;

	case CXType_ConstantArray: {
		entry->second.unexposed = type_is_unexposed_help(clang_getArrayElementType(type));
	} break;

	case CXType_Record: {
		bool unexposed = false;
		clang_Type_visitFields_NotBroken(type, [](CXCursor c, CXClientData data) -> CXVisitorResult {
			bool* unexposed = (bool*)data;
			*unexposed = *unexposed || type_is_unexposed_help(clang_getCursorType(c));
			return CXVisit_Continue;
		}, &unexposed);
		entry->second.unexposed = unexposed;
	} break;

	case CXType_FunctionProto: {
		bool unexposed = type_is_unexposed_help(clang_getResultType(type));
		for(int i = 0; i < clang_getNumArgTypes(type); i++) {
			unexposed = unexposed || type_is_unexposed_help(clang_getArgType(type, i));
		}
		entry->second.unexposed = unexposed;
	} break;

	default: break;
	}

	return entry->second.unexposed;
}

bool type_is_unexposed(CXType type) {

	bool result = type_is_unexposed_help(type);
	clear_in_progress_unx();
	return result;
}

int count_enum_fields(CXType type) {

	CXCursor decl = clang_getTypeDeclaration(type);

	int num = 0;
	clang_visitChildren(decl, [](CXCursor c, CXCursor parent, CXClientData data) -> CXChildVisitResult {
		if(c.kind == CXCursor_EnumConstantDecl) (*(int*)data)++;
		return CXChildVisit_Continue;
	}, &num);
	return num;
}

struct enum_field {
	std::string name;
	uint64_t val;
};

CXChildVisitResult print_enum_field(CXCursor c, CXCursor parent, CXClientData client_data) {

	std::vector<enum_field>* m = (std::vector<enum_field>*)client_data;

	if(c.kind == CXCursor_EnumConstantDecl) {

		uint64_t value = clang_getEnumConstantDeclUnsignedValue(c);
		std::string name = to_string(clang_getCursorSpelling(c));

		m->push_back({name, value});
	}

	return CXChildVisit_Continue;
}

void print_enum(CXType type) {

	CXCursor decl = clang_getTypeDeclaration(type);
	CXType underlying = clang_getEnumDeclIntegerType(decl);
	
	std::string name = to_string(clang_getCursorSpelling(decl));
	std::string type_name = to_string(clang_getTypeSpelling(type));
	std::string underlying_name = to_string(clang_getTypeSpelling(underlying));

	if(count_enum_fields(type) > 256) {
		log_out << "WARN: enum " << type_name << " has too many fields, skipping!" << std::endl;
	}

	std::vector<enum_field> members;

	clang_visitChildren(decl, print_enum_field, &members);

	fout << "template<> struct Type_Info<" << type_name << "> {" << std::endl
		 << "\tstatic constexpr char name[] = \"" << type_name << "\";" << std::endl
		 << "\tstatic constexpr usize size = sizeof(" << type_name << ");" << std::endl
		 << "\tstatic constexpr Type_Type type = Type_Type::enum_;" << std::endl
		 << "\tstatic constexpr usize count = " << members.size() << ";" << std::endl
		 << "\tusing underlying = " << underlying_name << ";" << std::endl;

	std::string member_str;
	for(int i = 0; i < members.size(); i++) {
		fout << "\tstatic constexpr char __" << i << "[] = \"" << members[i].name << "\";" << std::endl; 
		member_str += "Enum_Field<" + std::to_string(members[i].val) + ", __" + std::to_string(i) + ">";
		if(i != members.size() - 1) member_str += ", ";
	}

	fout << "\tusing members = Type_List<" << member_str << ">;" << std::endl
	 	 << "};" << std::endl;
}

struct field_client {
	int i = 0;
	std::string outer;
	bool unexposed = false;
};

CXVisitorResult print_field(CXCursor c, CXClientData client_data) {

	field_client* data = (field_client*)client_data;

	CXType type = clang_getCursorType(c);

	std::string name  = to_string(clang_getCursorSpelling(c));
	std::string type_name = to_string(clang_getTypeSpelling(type));

	std::string total = "&" + data->outer + "::" + name;

	fout << "\t\tthis_type_info._struct.member_names[" << data->i << "] = \"" << name << "\"_;" << std::endl;

	if(data->unexposed)
		fout << "\t\tthis_type_info._struct.member_types[" << data->i << "] = TYPEINFO_GET_HASH(decltype(get_member_type(" << total << ")));" << std::endl
			 << "\t\tthis_type_info._struct.member_offsets[" << data->i << "] = (u32)offset_of<decltype(get_class_type(" << total << ")),decltype(get_member_type(" << total << "))," << total << ">();" << std::endl;
	else
		fout << "\t\tthis_type_info._struct.member_types[" << data->i << "] = TYPEINFO_GET_HASH(" << type_name << ");" << std::endl
			 << "\t\tthis_type_info._struct.member_offsets[" << data->i << "] = offsetof(" << data->outer << ", " << name << ");" << std::endl;

	data->i++;
	return CXVisit_Continue;
}

CXChildVisitResult check_noreflect(CXCursor c, CXCursor parent, CXClientData client_data) {

	bool* data = (bool*)client_data;

	if(c.kind == CXCursor_AnnotateAttr) {
		
		std::string attribute = to_string(clang_getCursorSpelling(c));

		if(attribute == "noreflect") {
			*data = true;
			log_out << "type is noreflect" << std::endl;
			return CXChildVisit_Break;
		}
	}

	return CXChildVisit_Continue;
}

int count_fields(CXType type) {

	int num = 0;
	clang_Type_visitFields_NotBroken(type, [](CXCursor c, CXClientData data) -> CXVisitorResult {
		(*(int*)data)++;
		return CXVisit_Continue;
	}, &num);
	return num;
}

void print_record(CXType type, bool just_print = false) {

	CXCursor decl = clang_getTypeDeclaration(type);

	if(clang_Cursor_isAnonymous(decl)) return;

	while(clang_getCursorType(decl).kind == CXType_Unexposed) {
		decl = clang_getSpecializedCursorTemplate(decl);
	}
	
	bool no_reflect = false;
	clang_visitChildren(decl, check_noreflect, &no_reflect);

	if(no_reflect) return;

	if(!just_print) {
		clang_Type_visitFields_NotBroken(type, [](CXCursor c, CXClientData data) -> CXVisitorResult {
			
			CXType field = clang_getCursorType(c);
			CXCursor decl = clang_getTypeDeclaration(field);
			if(!(field.kind == CXType_Record && clang_Cursor_isAnonymous(decl))) {
				print_dep_type(field, *(CXType*)data);
			}
			return CXVisit_Continue;
		}, &type);
	}

	std::string name = to_string(clang_getCursorSpelling(decl));
	std::string type_name = to_string(clang_getTypeSpelling(type));

	if(count_fields(type) > 96) {
		log_out << "WARN: struct " << type_name << " has too many fields, skipping!" << std::endl;
	}

	// fout << "\t[]() -> void {" << std::endl
	// 	 << "\t\t_type_info this_type_info;" << std::endl
	// 	 << "\t\tthis_type_info.type_type = Type::_struct;" << std::endl
	// 	 << "\t\tthis_type_info.size = sizeof(" << type_name << ");" << std::endl
	// 	 << "\t\tthis_type_info.name = \"" << name << "\"_;" << std::endl
	// 	 << "\t\tthis_type_info.hash = (type_id)typeid(" << type_name << ").hash_code();" << std::endl;

	// field_client data;
	// data.outer = type_name;
	// data.unexposed = type.kind == CXType_Unexposed;

	// clang_Type_visitFields_NotBroken(type, print_field, &data);
	
	// fout << "\t\tthis_type_info._struct.member_count = " << data.i << ";" << std::endl
	// 	 << "\t\t_type_info* val = type_table.get_or_insert_blank(this_type_info.hash);" << std::endl
	// 	 << "\t\t*val = this_type_info;" << std::endl
	// 	 << "\t}();" << std::endl << std::endl;
}

void print_func_pointer(CXType type, bool just_print = false) {

	CXType func = clang_getPointeeType(type);
	CXType ret = clang_getResultType(func);

	if(!just_print) {
		print_dep_type(ret, func);
		for(int i = 0; i < clang_getNumArgTypes(func); i++) {
			CXType param = clang_getArgType(func, i);
			print_dep_type(param, func);
		}
	}

	std::string ptr_type_name = to_string(clang_getTypeSpelling(type));
	std::string func_type_name = to_string(clang_getTypeSpelling(func));
	std::string return_type_name = to_string(clang_getTypeSpelling(ret));

	if(clang_getNumArgTypes(func) > 16) {
		log_out << "WARN: function pointer " << func_type_name << " has too many arguments, skipping!" << std::endl;
	}

	// fout << "\t[]() -> void {" << std::endl
	// 	 << "\t\t_type_info this_type_info;" << std::endl
	// 	 << "\t\tthis_type_info.type_type = Type::_func;" << std::endl
	// 	 << "\t\tthis_type_info.size = sizeof(" << ptr_type_name << ");" << std::endl
	// 	 << "\t\tthis_type_info.hash = (type_id)typeid(" << ptr_type_name << ").hash_code();" << std::endl
	// 	 << "\t\tthis_type_info.name = \"" << func_type_name << "\"_;" << std::endl
	// 	 << "\t\tthis_type_info._func.signature = \"" << func_type_name << "\"_;" << std::endl
	// 	 << "\t\tthis_type_info._func.return_type = TYPEINFO_GET_HASH(" << return_type_name << ");" << std::endl
	// 	 << "\t\tthis_type_info._func.param_count = " << clang_getNumArgTypes(func) << ";" << std::endl;

	// for(int i = 0; i < clang_getNumArgTypes(func); i++) {
		
	// 	CXType param = clang_getArgType(func, i);
	// 	std::string param_type_name = to_string(clang_getTypeSpelling(param));

	// 	fout << "\t\tthis_type_info._func.param_types[" << i << "] = TYPEINFO_GET_HASH(" << param_type_name << ");" << std::endl;
	// }

	// fout << "\t\t_type_info* val = type_table.get_or_insert_blank(this_type_info.hash);" << std::endl
	// 	 << "\t\t*val = this_type_info;" << std::endl
	// 	 << "\t}();" << std::endl << std::endl;	
}

void print_pointer(CXType type, bool just_print = false) {

	CXType to = clang_getPointeeType(type);

	if(clang_getNumArgTypes(type) != -1) {
		print_func_pointer(type, just_print);
		return;
	}

	if(!just_print) {
		print_dep_type(to, type);
		if(to.kind == CXType_Record) {
			bool no_reflect = false;
			clang_visitChildren(clang_getTypeDeclaration(to), check_noreflect, &no_reflect);
			if(no_reflect) return;
		}
	}

	std::string type_name = to_string(clang_getTypeSpelling(type));
	std::string to_name = to_string(clang_getTypeSpelling(to));

	// dumb workaround
	if(to_name == "unsigned char") return;

	fout << "template<> struct Type_Info<" << type_name << "> {" << std::endl
		 << "\tstatic constexpr char name[] = \"" << to_name << "*\";" << std::endl
		 << "\tstatic constexpr usize size = sizeof(" << type_name << ");" << std::endl
		 << "\tstatic constexpr Type_Type type = Type_Type::ptr_;" << std::endl
		 << "\tusing to = " << to_name << ";" << std::endl
		 << "};" << std::endl;
}

void print_array(CXType type, bool just_print = false) {

	CXType underlying = clang_getArrayElementType(type);
	int64_t length = clang_getArraySize(type);

	if(!just_print) {
		print_dep_type(underlying, type);
	}

	std::string type_name = to_string(clang_getTypeSpelling(type));
	std::string base_type_name = to_string(clang_getTypeSpelling(underlying));

	fout << "template<> struct Type_Info<" << type_name << "> {" << std::endl
		 << "\tstatic constexpr char name[] = \"" << type_name << "\";" << std::endl
		 << "\tstatic constexpr usize size = sizeof(" << type_name << ");" << std::endl
		 << "\tstatic constexpr Type_Type type = Type_Type::array_;" << std::endl
		 << "\tstatic constexpr usize len = " << length << ";" << std::endl
		 << "\tusing underlying = " << base_type_name << ";" << std::endl
		 << "};" << std::endl;
}

bool type_is_not_specified(CXType type) {

	while(type.kind == CXType_Pointer) type = clang_getPointeeType(type);

	CXCursor declc = clang_getTypeDeclaration(type);
	if(declc.kind == CXCursor_NoDeclFound) return true;

	CXPrintingPolicy policy = clang_getCursorPrintingPolicy(declc);

	clang_PrintingPolicy_setProperty(policy, CXPrintingPolicy_FullyQualifiedName, true);

	std::string pretty = to_string(clang_getCursorPrettyPrinted(declc, policy));

	bool result = false;
	if(pretty.length() > 10 && pretty.substr(0, 10) == "template <" && pretty[10] != '>') result = true;

	clang_PrintingPolicy_dispose(policy);

	return result;
}

void print_data_type_help(CXType type, CXType parent) {

	auto entry = types.find(type);
	if(entry == types.end()) return;
	type_data& data = entry->second;

	if(data.in_progress_out) {
		back_patches.push_back(parent);
		return;
	}
	if(data.done) {
		log_out << "Already seen type: " << clang_getTypeSpelling(type) << std::endl;
		return;
	}
	data.in_progress_out = true;

	log_out << "Type: " << clang_getTypeSpelling(type) << std::endl
			<< "\treferences: ";
	for(CXCursor c : data.references) {
		CXSourceLocation loc = clang_getCursorLocation(c);
		CXFile file; unsigned int line;
		clang_getExpansionLocation(loc, &file, &line, nullptr, nullptr);
		log_out << clang_getFileName(file) << ":" << line << ", ";
	}
	log_out << std::endl;

	if(type_is_unexposed(type)) {
		log_out << "\ttype is unexposed" << std::endl;
		if(type_is_not_specified(type)) {
			log_out << "\ttype is not instantiated, continuing" << std::endl;
			data.done = true;
			return;
		}
	}

	switch(type.kind) {
	
	case CXType_Enum: print_enum(type); break;
	case CXType_Unexposed:
	case CXType_Record: print_record(type); break;
	case CXType_ConstantArray: print_array(type); break;
	case CXType_Pointer: print_pointer(type); break;

	case CXType_Elaborated: {
		CXType named = clang_Type_getNamedType(type);
		print_dep_type(named, type);
	} break;

	default: {
		log_out << "\tkind: " << clang_getTypeKindSpelling(type.kind) << std::endl; 
	} break;
	}

	data.done = true;
}

void resolve_patches() {
	for(CXType type : back_patches) {
		switch(type.kind) {
		case CXType_Record: print_record(type, true); break;
		case CXType_ConstantArray: print_array(type, true); break;
		case CXType_Pointer: print_pointer(type, true); break;
		default: break;
		}
	}
	back_patches.clear();
}

void print_data_type(CXType type) {

	if(done.find(type) != done.end()) return;
	if(done.find(clang_getCanonicalType(type)) != done.end()) return;

	print_data_type_help(type, type);
	clear_in_progress_out();
	// resolve_patches();

	done.insert(type);
	done.insert(clang_getCanonicalType(type));
}

void print_results() {

	for(auto& entry : types) {
		ensure_deps_in_graph(entry.first);
	}
	for(auto& entry : types) {
		print_data_type(entry.first);
	}
}

void print_basic_types() {

	fout << R"STR(
template<> struct Type_Info<void> {
	static constexpr char name[] = "void";
	static constexpr usize size = 0u;
	static constexpr Type_Type type = Type_Type::void_;
};
template<> struct Type_Info<decltype(nullptr)> {
	static constexpr char name[] = "nullptr";
	static constexpr usize size = sizeof(nullptr);
	static constexpr Type_Type type = Type_Type::ptr_;
	using to = void;
};
template<> struct Type_Info<char> {
	static constexpr char name[] = "char";
	static constexpr usize size = sizeof(char);
	static constexpr bool sgn = true;
	static constexpr Type_Type type = Type_Type::int_;
};
template<> struct Type_Info<i8> {
	static constexpr char name[] = "i8";
	static constexpr usize size = sizeof(i8);
	static constexpr bool sgn = true;
	static constexpr Type_Type type = Type_Type::int_;
};
template<> struct Type_Info<u8> {
	static constexpr char name[] = "u8";
	static constexpr usize size = sizeof(u8);
	static constexpr bool sgn = false;
	static constexpr Type_Type type = Type_Type::int_;
};
template<> struct Type_Info<i16> {
	static constexpr char name[] = "i16";
	static constexpr usize size = sizeof(i16);
	static constexpr bool sgn = true;
	static constexpr Type_Type type = Type_Type::int_;
};
template<> struct Type_Info<u16> {
	static constexpr char name[] = "u16";
	static constexpr usize size = sizeof(u16);
	static constexpr bool sgn = false;
	static constexpr Type_Type type = Type_Type::int_;
};
template<> struct Type_Info<i32> {
	static constexpr char name[] = "i32";
	static constexpr usize size = sizeof(i32);
	static constexpr bool sgn = true;
	static constexpr Type_Type type = Type_Type::int_;
};
template<> struct Type_Info<u32> {
	static constexpr char name[] = "u32";
	static constexpr usize size = sizeof(u32);
	static constexpr bool sgn = false;
	static constexpr Type_Type type = Type_Type::int_;
};
template<> struct Type_Info<i64> {
	static constexpr char name[] = "i64";
	static constexpr usize size = sizeof(i64);
	static constexpr bool sgn = true;
	static constexpr Type_Type type = Type_Type::int_;
};
template<> struct Type_Info<u64> {
	static constexpr char name[] = "u64";
	static constexpr usize size = sizeof(u64);
	static constexpr bool sgn = false;
	static constexpr Type_Type type = Type_Type::int_;
};
template<> struct Type_Info<f32> {
	static constexpr char name[] = "f32";
	static constexpr usize size = sizeof(f32);
	static constexpr Type_Type type = Type_Type::float_;
};
template<> struct Type_Info<f64> {
	static constexpr char name[] = "f64";
	static constexpr usize size = sizeof(f64);
	static constexpr Type_Type type = Type_Type::float_;
};
template<> struct Type_Info<bool> {
	static constexpr char name[] = "bool";
	static constexpr usize size = sizeof(bool);
	static constexpr Type_Type type = Type_Type::bool_;
};
template<> struct Type_Info<string> {
	static constexpr char name[] = "string";
	static constexpr usize size = sizeof(string);
	static constexpr Type_Type type = Type_Type::string_;
};
	)STR" << std::endl;
}

void print_header(std::string in_file) {

fout << "#define COMPILING_META_TYPES" << std::endl
				 << "#include \"" << in_file << "\"" << std::endl << std::endl
				 << R"STR(template <typename T, typename M> M get_member_type(M T::*);
template <typename T, typename M> T get_class_type(M T::*);
template <typename T, typename R, R T::*M>
constexpr u64 offset_of() {
    return reinterpret_cast<u64>(&(((T*)0)->*M));
})STR" << std::endl;
}

int main(int argc, char** argv) {

	if(argc < 3) {
		std::cout << "Must provide input and output file." << std::endl;
		return -1;
	}

	std::string in_file(argv[1]);
	std::string out_file(argv[2]);
	
	log_out.open("meta.log");
	fout.open(out_file);

	// Parse input
	{
		CXIndex index = clang_createIndex(0, 0);
		CXTranslationUnit unit = clang_parseTranslationUnit(index, in_file.c_str(), argv + 3, argc - 3, nullptr, 0, 
			CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing);

		if (unit == nullptr) {
			std::cout << "Unable to parse translation unit." << std::endl;
			return -1;
		}

	    for (unsigned int i = 0; i < clang_getNumDiagnostics(unit); i++) {
	        std::string msg = to_string(clang_getDiagnosticSpelling(clang_getDiagnostic(unit, i)));
	        if(msg != "too many errors emitted, stopping now")
	        	log_out << "DIAG: " << msg << std::endl;
	    }

		CXCursor cursor = clang_getTranslationUnitCursor(unit);

		clang_visitChildren(cursor, traversal, nullptr);

		{ // Do output 
			print_header(in_file);
			print_basic_types();
			print_results();
		}

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}

	return 0;
}
