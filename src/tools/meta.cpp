
#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <unordered_map>
#include <algorithm>

#include <clang-c/Index.h>

std::ofstream log_out;

std::ostream& operator<<(std::ostream& stream, const CXString& str) {
	stream << clang_getCString(str);
	clang_disposeString(str);
	return stream;
}

std::string to_string(CXString cx_str) {
	std::string str(clang_getCString(cx_str));
	clang_disposeString(cx_str);
	return str;
}

void print_basic_types(std::ofstream& fout) {

	fout << R"STR(
	{
		_type_info void_t;
		void_t.type_type 		= Type::_void;
		void_t.size				= 0;
		void_t.name 			= "void"_;
		void_t._int.is_signed 	= true;
		void_t.hash = (type_id)typeid(void).hash_code();
		type_table.insert(void_t.hash, void_t, false);
	}
	{
		_type_info char_t;
		char_t.type_type 		= Type::_int;
		char_t.size				= sizeof(char);
		char_t.name 			= "char"_;
		char_t._int.is_signed 	= true;
		char_t.hash = (type_id)typeid(char).hash_code();
		type_table.insert(char_t.hash, char_t, false);
	}
	{
		_type_info u8_t;
		u8_t.type_type 		= Type::_int;
		u8_t.size			= sizeof(u8);
		u8_t.name 			= "u8"_;
		u8_t._int.is_signed = false;
		u8_t.hash = (type_id)typeid(u8).hash_code();
		type_table.insert(u8_t.hash, u8_t, false);
	}
	{
		_type_info i8_t;
		i8_t.type_type 		= Type::_int;
		i8_t.size			= sizeof(i8);
		i8_t.name 			= "i8"_;
		i8_t._int.is_signed = true;
		i8_t.hash = (type_id)typeid(i8).hash_code();
		type_table.insert(i8_t.hash, i8_t, false);
	}
	{
		_type_info u16_t;
		u16_t.type_type 		= Type::_int;
		u16_t.size				= sizeof(u16);
		u16_t.name 				= "u16"_;
		u16_t._int.is_signed 	= false;
		u16_t.hash = (type_id)typeid(u16).hash_code();
		type_table.insert(u16_t.hash, u16_t, false);
	}
	{
		_type_info i16_t;
		i16_t.type_type 		= Type::_int;
		i16_t.size				= sizeof(i16);
		i16_t.name 				= "i16"_;
		i16_t._int.is_signed 	= true;
		i16_t.hash = (type_id)typeid(i16).hash_code();
		type_table.insert(i16_t.hash, i16_t, false);
	}
	{
		_type_info u32_t;
		u32_t.type_type 		= Type::_int;
		u32_t.size				= sizeof(u32);
		u32_t.name 				= "u32"_;
		u32_t._int.is_signed 	= false;
		u32_t.hash = (type_id)typeid(u32).hash_code();
		type_table.insert(u32_t.hash, u32_t, false);
	}
	{
		_type_info i32_t;
		i32_t.type_type 		= Type::_int;
		i32_t.size				= sizeof(i32);
		i32_t.name 				= "i32"_;
		i32_t._int.is_signed 	= true;
		i32_t.hash = (type_id)typeid(i32).hash_code();
		type_table.insert(i32_t.hash, i32_t, false);
	}
	{
		_type_info u64_t;
		u64_t.type_type 		= Type::_int;
		u64_t.size				= sizeof(u64);
		u64_t.name 				= "u64"_;
		u64_t._int.is_signed 	= false;
		u64_t.hash = (type_id)typeid(u64).hash_code();
		type_table.insert(u64_t.hash, u64_t, false);
	}
	{
		_type_info i64_t;
		i64_t.type_type 		= Type::_int;
		i64_t.size				= sizeof(i64);
		i64_t.name 				= "i64"_;
		i64_t._int.is_signed 	= true;
		i64_t.hash = (type_id)typeid(i64).hash_code();
		type_table.insert(i64_t.hash, i64_t, false);
	}
	{
		_type_info f32_t;
		f32_t.type_type 		= Type::_float;
		f32_t.size				= sizeof(f32);
		f32_t.name 				= "f32"_;
		f32_t._int.is_signed 	= true;
		f32_t.hash = (type_id)typeid(f32).hash_code();
		type_table.insert(f32_t.hash, f32_t, false);
	}
	{
		_type_info f64_t;
		f64_t.type_type 		= Type::_float;
		f64_t.size				= sizeof(f64);
		f64_t.name 				= "f64"_;
		f64_t._int.is_signed 	= true;
		f64_t.hash = (type_id)typeid(f64).hash_code();
		type_table.insert(f64_t.hash, f64_t, false);
	}
	{
		_type_info bool_t;
		bool_t.type_type 		= Type::_bool;
		bool_t.size				= sizeof(bool);
		bool_t.name 			= "bool"_;
		bool_t._int.is_signed 	= true;
		bool_t.hash = (type_id)typeid(bool).hash_code();
		type_table.insert(bool_t.hash, bool_t, false);
	}
	{
		_type_info string_t;
		string_t.type_type 		= Type::_string;
		string_t.size			= sizeof(string);
		string_t.name 			= "string"_;
		string_t.hash = (type_id)typeid(string).hash_code();
		type_table.insert(string_t.hash, string_t, false);
	}
	)STR" << std::endl;
}

enum class def_types {
	_enum,
	_struct
};

bool operator==(const CXType& l, const CXType& r) {
	return clang_equalTypes(l, r);
}

struct data_type_enum {
	struct member {
		std::string name;
		uint64_t value;
	};
	bool is_class;
	CXType underlying_type;
	std::vector<member> members;
};

struct data_type_struct {
	struct field {
		std::string name;
		CXType id;
		CXCursor cursor;
	};
	bool no_reflect = false;
	std::vector<field> fields;
};

struct data_type {
	def_types type;
	
	std::string name;
	CXType my_type;
	bool output = false;

	data_type_enum _enum;
	data_type_struct _struct;
};

namespace std {
	template <>
	struct hash<CXType> {
		std::size_t operator()(const CXType& k) const {
			return hash<string>()(to_string(clang_getTypeSpelling(k)));
		}
	};
}

struct g_parse_data {
	bool parsing = false;
	int recurse_level = 0;
	CXCursor top_level;
	std::unordered_map<CXType, data_type> type_graph;
	std::stack<CXType> current;
};
g_parse_data g_data;

struct idt{};
std::ostream& operator<<(std::ostream& stream, const idt&) {
	for(int i = 0; i < g_data.recurse_level; i++) stream << "\t";
	return stream;
}

void print_data_type(std::ofstream& fout, data_type& dt);
CXChildVisitResult traversal_enum(CXCursor c, CXCursor parent, CXClientData client_data);
CXChildVisitResult traversal_struct(CXCursor c, CXCursor parent, CXClientData client_data);

CXChildVisitResult parse_enum(CXCursor c, CXCursor parent, CXClientData client_data) {

	CXCursorKind kind = c.kind;
	log_out << idt{} << "Parsing Cursor: " << clang_getCursorSpelling(c) << std::endl
			<< idt{} << "Kind: " << clang_getCursorKindSpelling(kind) << std::endl;

	data_type& type = g_data.type_graph[g_data.current.top()];

	switch(kind) {
	case CXCursor_EnumConstantDecl: {
		data_type_enum::member member;
		member.value = clang_getEnumConstantDeclUnsignedValue(c);
		member.name  = to_string(clang_getCursorSpelling(c));

		log_out << idt{} << "name: " << member.name << std::endl
				<< idt{} << "value: " << member.value << std::endl;

		type._enum.members.push_back(member);
	} break;
	}

	return CXChildVisit_Continue;
}

CXChildVisitResult parse_struct(CXCursor c, CXCursor parent, CXClientData client_data) {

	CXCursorKind kind = c.kind;
	log_out << idt{} << "Parsing Cursor: " << clang_getCursorSpelling(c) << std::endl
			<< idt{} << "Kind: " << clang_getCursorKindSpelling(kind) << std::endl;

	data_type& type = g_data.type_graph[g_data.current.top()];

	switch(kind) {
	case CXCursor_FieldDecl: {
		data_type_struct::field field;
		field.name  = to_string(clang_getCursorSpelling(c));
		field.id = clang_getCursorType(c);
		field.cursor = c;

		log_out << idt{} << "name: " << field.name << std::endl
				<< idt{} << "type: " << clang_getTypeSpelling(field.id) << std::endl;

		type._struct.fields.push_back(field);
	} break;
	case CXCursor_EnumDecl: {
		if(!clang_Cursor_isAnonymous(c))
			traversal_enum(c, parent, client_data);
	} break;
	case CXCursor_UnionDecl:
	case CXCursor_ClassDecl:
	case CXCursor_StructDecl: {
		if(clang_Cursor_isAnonymous(c)) {
			clang_visitChildren(c, parse_struct, client_data);
		} else {
			traversal_struct(c, parent, client_data);
		}
	} break;
	case CXCursor_AnnotateAttr: {
		std::string annotation = to_string(clang_getCursorSpelling(c));
		if(annotation == "noreflect") {
			type._struct.no_reflect = true;
		}
	} break;
	}

	return CXChildVisit_Continue;
}

bool cursor_is_record_def(CXCursor c) {
	if(!clang_isCursorDefinition(c)) return false;
	CXCursorKind kind = c.kind;
	return kind == CXCursor_StructDecl ||
		   kind == CXCursor_UnionDecl  ||
		   kind == CXCursor_ClassDecl  ||
		   kind == CXCursor_EnumDecl;
}

bool cursor_is_type_accessible(CXCursor c) {
	while(!clang_equalCursors(c, g_data.top_level)) {
		if(!cursor_is_record_def(c)) {
			return false;
		}
		c = clang_getCursorLexicalParent(c);
	}
	return true;
}

CXChildVisitResult traversal_struct(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(!clang_isCursorDefinition(c)) {
		log_out << idt{} << "Forward declaration; continuing" << std::endl;	
		return CXChildVisit_Continue;
	} else if(!cursor_is_type_accessible(c)) {
		log_out << idt{} << "Enum declaration inaccessible; continuing" << std::endl;
		return CXChildVisit_Continue;
	}

	std::string name = to_string(clang_getCursorSpelling(c));
	log_out << idt{} << "Parsing struct definition " << name << std::endl;

	data_type type;
	type.type = def_types::_struct;
	type.name = name;
	type.my_type = clang_getCursorType(c);

	log_out << idt{} << "Name: " << type.name << std::endl
			<< idt{} << "my_type: " << clang_getTypeSpelling(type.my_type) << std::endl;

	g_data.type_graph.insert({type.my_type, type});
	g_data.current.push(type.my_type);

	g_data.recurse_level++;
	clang_visitChildren(c, parse_struct, client_data);
	g_data.recurse_level--;

	g_data.current.pop();

	return CXChildVisit_Continue;
}

CXChildVisitResult traversal_enum(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(!clang_isCursorDefinition(c)) {
		log_out << idt{} << "Forward declaration; continuing" << std::endl;	
		return CXChildVisit_Continue;
	} else if(!cursor_is_type_accessible(c)) {
		log_out << idt{} << "Enum declaration inaccessible; continuing" << std::endl;
		return CXChildVisit_Continue;
	}

	std::string name = to_string(clang_getCursorSpelling(c));
	log_out << idt{} << "Parsing enum definition " << name << std::endl;

	data_type type;
	type.type = def_types::_enum;
	type.name = name;
	type.my_type = clang_getCursorType(c);

	type._enum.is_class = clang_EnumDecl_isScoped(c);
	type._enum.underlying_type = clang_getEnumDeclIntegerType(c);

	log_out << idt{} << "name: " << type.name << std::endl
			<< idt{} << "is_class: " << type._enum.is_class << std::endl
			<< idt{} << "underlying_type: " << clang_getTypeSpelling(type._enum.underlying_type) << std::endl
			<< idt{} << "my_type: " << clang_getTypeSpelling(type.my_type) << std::endl;

	g_data.type_graph.insert({type.my_type, type});
	g_data.current.push(type.my_type);

	g_data.recurse_level++;
	clang_visitChildren(c, parse_enum, client_data);
	g_data.recurse_level--;

	g_data.current.pop();

	return CXChildVisit_Continue;
}

CXChildVisitResult traversal(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(clang_Location_isInSystemHeader(clang_getCursorLocation(c))) {
		return CXChildVisit_Continue;
	} else if(!g_data.parsing) {
		if(c.kind == CXCursor_EnumDecl && to_string(clang_getCursorSpelling(c)) == "META_START_PARSING_HERE") 
			g_data.parsing = true;
		return CXChildVisit_Continue;
	}

	CXCursorKind kind = c.kind;
	log_out << idt{} << "Parsing Cursor: " << clang_getCursorSpelling(c) << std::endl
			<< idt{} << "Kind: " << clang_getCursorKindSpelling(kind) << std::endl;

	if(clang_isPreprocessing(kind)) {

		log_out << idt{} << "Preprocessor; continuing" << std::endl;
		return CXChildVisit_Continue;

	} else if(clang_isUnexposed(kind)) {

		log_out << idt{} << "Unexposed; continuing" << std::endl;
		return CXChildVisit_Continue;

	} else if(clang_isInvalid(kind)) {
	
		log_out << idt{} << "Invalid; continuing" << std::endl;	
		return CXChildVisit_Continue;
	}

	g_data.recurse_level++;

	CXChildVisitResult result = CXChildVisit_Recurse;

	switch(kind) {
	case CXCursor_EnumDecl: result = traversal_enum(c, parent, client_data); break;
	case CXCursor_UnionDecl:
	case CXCursor_ClassDecl:
	case CXCursor_StructDecl: result = traversal_struct(c, parent, client_data); break;
	}

	g_data.recurse_level--;	

	return result;
}

void print_struct(std::ofstream& fout, data_type& dt) {

	if(dt.output) {
		log_out << "OUTPUT: already seen struct " << dt.name << std::endl;
		return;
	}

	data_type_struct& type = dt._struct;

	for(auto field : type.fields) {
		auto entry = g_data.type_graph.find(field.id);
		if(entry != g_data.type_graph.end()) {
			print_data_type(fout, entry->second);
		}
	}

	log_out << "OUTPUT: struct " << dt.name << std::endl;

	if(type.no_reflect) {
		dt.output = true;
		return;
	}

	std::string type_name = to_string(clang_getTypeSpelling(dt.my_type));

	fout << "\t[]() -> void {" << std::endl
		 << "\t\t_type_info this_type_info;" << std::endl
		 << "\t\tthis_type_info.type_type = Type::_struct;" << std::endl
		 << "\t\tthis_type_info.size = " << clang_Type_getSizeOf(dt.my_type) << ";" << std::endl
		 << "\t\tthis_type_info.name = \"" << dt.name << "\"_;" << std::endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << type_name << ").hash_code();" << std::endl;

	int idx = 0;
	for(auto field : type.fields) {
		std::string member_type = to_string(clang_getTypeSpelling(field.id));

		fout << "\t\tthis_type_info._struct.member_types[" << idx << "] = TYPEINFO(" << member_type << ") ? TYPEINFO(" << member_type << ")->hash : 0;" << std::endl
			 << "\t\tthis_type_info._struct.member_names[" << idx << "] = \"" << field.name << "\"_;" << std::endl
			 << "\t\tthis_type_info._struct.member_offsets[" << idx << "] = " << clang_Type_getOffsetOf(dt.my_type, field.name.c_str()) / 8 << ";" << std::endl;
		idx++;
	}

	fout << "\t\tthis_type_info._struct.member_count = " << idx << ";" << std::endl;

	fout << "\t\ttype_table.insert(this_type_info.hash, this_type_info, false);" << std::endl
		 << "\t}();" << std::endl << std::endl;

	dt.output = true;
}

void print_enum(std::ofstream& fout, data_type& dt) {

	if(dt.output) {
		log_out << "OUTPUT: already seen enum " << dt.name << std::endl;
		return;
	}

	data_type_enum& type = dt._enum;

	std::string underlying_name = to_string(clang_getTypeSpelling(type.underlying_type));
	log_out << "OUTPUT: enum " << dt.name << " : " << underlying_name << std::endl;

	if(type.members.size() > 256) {
		log_out << "WARNING: enum " << dt.name << " has too many members, skipping!" << std::endl;
		return;
	}

	std::string type_name = to_string(clang_getTypeSpelling(dt.my_type));

	fout << "\t[]() -> void {" << std::endl
		 << "\t\t_type_info this_type_info;" << std::endl
		 << "\t\tthis_type_info.type_type = Type::_enum;" << std::endl
		 << "\t\tthis_type_info.size = " << clang_Type_getSizeOf(dt.my_type) << ";" << std::endl
		 << "\t\tthis_type_info.name = \"" << dt.name << "\"_;" << std::endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << type_name << ").hash_code();" << std::endl
		 << "\t\tthis_type_info._enum.member_count = " << type.members.size() << ";" << std::endl
		 << "\t\tthis_type_info._enum.base_type = TYPEINFO(" << underlying_name << ") ? TYPEINFO(" << underlying_name << ")->hash : 0;" << std::endl;

	std::stable_sort(type.members.begin(), type.members.end(), 
		[](const data_type_enum::member& l, const data_type_enum::member& r) -> bool {return l.value < r.value;});

	int idx = 0;
	for(auto member : type.members) {
		fout << "\t\tthis_type_info._enum.member_names[" << idx << "] = \"" << member.name << "\"_;" << std::endl
			 << "\t\tthis_type_info._enum.member_values[" << idx << "] = " << member.value << ";" << std::endl;
		idx++;
	}

	fout << "\t\ttype_table.insert(this_type_info.hash, this_type_info, false);" << std::endl
		 << "\t}();" << std::endl << std::endl;

	dt.output = true;
}

void print_data_type(std::ofstream& fout, data_type& dt) {
	switch(dt.type) {
	case def_types::_enum: print_enum(fout, dt); break;
	case def_types::_struct: print_struct(fout, dt); break;
	} 
}

void print_results(std::ofstream& fout) {

	for(auto& entry : g_data.type_graph) {
		print_data_type(fout, entry.second);
	}
}

int main(int argc, char** argv) {

	if(argc < 3) {
		std::cout << "Must provide input and output file." << std::endl;
		return -1;
	}

	std::string in_file(argv[1]);
	std::string out_file(argv[2]);
	log_out.open("meta.log");

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
	        CXDiagnostic diagnostic = clang_getDiagnostic(unit, i);
	        log_out << "DIAG: " << clang_getDiagnosticSpelling(diagnostic) << std::endl;
	    }

		CXCursor cursor = clang_getTranslationUnitCursor(unit);
		g_data.top_level = cursor;

		clang_visitChildren(cursor, traversal, nullptr);

		{ // Do output 
			std::ofstream fout(out_file);

			fout << "#define COMPILING_META_TYPES" << std::endl
				 << "#include \"" << in_file << "\"" << std::endl << std::endl
				 << "void make_meta_info() {  " << std::endl;

			print_basic_types(fout);
			print_results(fout);

			fout << "}" << std::endl << std::endl;
		}

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}

	return 0;
}
