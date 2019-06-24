
#include <iostream>
#include <fstream>
#include <string>
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
};

struct data_type_enum {
	struct member {
		std::string name;
		uint64_t value;
	};

	std::string name;
	bool is_class;
	CXType underlying_type;
	std::vector<member> members;
};

struct data_type {
	def_types type;
	
	data_type_enum _enum;
};

struct data_type_id {
	std::string name;
};
bool operator==(const data_type_id& l, const data_type_id& r) {
	return l.name == r.name;
}

namespace std {
	template <>
	struct hash<data_type_id> {
		std::size_t operator()(const data_type_id& k) const {
			return hash<string>()(k.name);
		}
	};
}

struct g_parse_data {
	int recurse_level = 0;
	std::unordered_map<data_type_id, data_type> type_graph;
	data_type_id current;
};
g_parse_data g_data;

struct do_indent{};
std::ostream& operator<<(std::ostream& stream, const do_indent&) {
	for(int i = 0; i < g_data.recurse_level; i++) stream << "\t";
	return stream;
}

CXChildVisitResult parse_enum(CXCursor c, CXCursor parent, CXClientData client_data) {

	CXCursorKind kind = c.kind;
	log_out << do_indent{} << "Parsing Cursor: " << clang_getCursorSpelling(c) << std::endl
			<< do_indent{} << "Kind: " << clang_getCursorKindSpelling(kind) << std::endl;
	g_data.recurse_level++;

	data_type& type = g_data.type_graph[g_data.current];

	switch(kind) {
	case CXCursor_EnumConstantDecl: {
		data_type_enum::member member;
		member.value = clang_getEnumConstantDeclUnsignedValue(c);
		member.name  = to_string(clang_getCursorSpelling(c));

		log_out << do_indent{} << "Name: " << member.name << std::endl
				<< do_indent{} << "value: " << member.value << std::endl;

		type._enum.members.push_back(member);
	} break;
	}

	g_data.recurse_level--;
	return CXChildVisit_Recurse;
}

CXChildVisitResult traversal_enum(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(!clang_isCursorDefinition(c)) {
		log_out << do_indent{} << "Forward declaration; continuing" << std::endl;	
		g_data.recurse_level--;
		return CXChildVisit_Continue;
	}

	std::string name = to_string(clang_getCursorSpelling(c));
	log_out << do_indent{} << "Parsing enum definition " << name << std::endl;

	data_type_id id{name};
	data_type type;
	type.type = def_types::_enum;

	type._enum.name = name;
	type._enum.is_class = clang_EnumDecl_isScoped(c);
	type._enum.underlying_type = clang_getEnumDeclIntegerType(c);

	log_out << do_indent{} << "Name: " << type._enum.name << std::endl
			<< do_indent{} << "is_class: " << type._enum.is_class << std::endl
			<< do_indent{} << "underlying_type: " << clang_getTypeSpelling(type._enum.underlying_type) << std::endl;

	g_data.type_graph.insert({id, type});
	g_data.current = id;

	clang_visitChildren(c, parse_enum, client_data);

	g_data.recurse_level--;
	return CXChildVisit_Continue;
}

CXChildVisitResult traversal(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(clang_Location_isInSystemHeader(clang_getCursorLocation(c))) {
		return CXChildVisit_Continue;
	}

	CXCursorKind kind = c.kind;
	log_out << do_indent{} << "Parsing Cursor: " << clang_getCursorSpelling(c) << std::endl
			<< do_indent{} << "Kind: " << clang_getCursorKindSpelling(kind) << std::endl;
	g_data.recurse_level++;

	if(clang_isPreprocessing(kind)) {

		log_out << do_indent{} << "Preprocessor; continuing" << std::endl;
		
		g_data.recurse_level--;
		return CXChildVisit_Continue;

	} else if(clang_isUnexposed(kind)) {

		log_out << do_indent{} << "Unexposed; continuing" << std::endl;

		g_data.recurse_level--;
		return CXChildVisit_Continue;

	} else if(clang_isInvalid(kind)) {
	
		log_out << do_indent{} << "Invalid; continuing" << std::endl;	

		g_data.recurse_level--;
		return CXChildVisit_Continue;
	}

	switch(kind) {
	case CXCursor_EnumDecl: return traversal_enum(c, parent, client_data);
	}

	g_data.recurse_level--;
	return CXChildVisit_Recurse;
}

void print_enum(std::ofstream& fout, data_type_enum type) {

	std::string underlying_name = to_string(clang_getTypeSpelling(type.underlying_type));
	log_out << "OUTPUT: enum " << type.name << " : " << underlying_name << std::endl;

	if(type.members.size() > 256) {
		log_out << "WARNING: enum " << type.name << " has too many members, skipping!" << std::endl;
		return;
	}

	fout << "\t[]() -> void {" << std::endl
		 << "\t\t_type_info this_type_info;" << std::endl
		 << "\t\tthis_type_info.type_type = Type::_enum;" << std::endl
		 << "\t\tthis_type_info.size = sizeof(" << type.name << ");" << std::endl
		 << "\t\tthis_type_info.name = \"" << type.name << "\"_;" << std::endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << type.name << ").hash_code();" << std::endl
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
}

void print_results(std::ofstream& fout) {

	for(auto entry : g_data.type_graph) {
		if(entry.second.type == def_types::_enum) {
			print_enum(fout, entry.second._enum);
		}
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
		auto index = clang_createIndex(0, 0);
		auto unit = clang_parseTranslationUnit(index, in_file.c_str(), argv + 3, argc - 3, nullptr, 0, 
			CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing);

		if (unit == nullptr) {
			std::cout << "Unable to parse translation unit." << std::endl;
			return -1;
		}

		auto cursor = clang_getTranslationUnitCursor(unit);
		clang_visitChildren(cursor, traversal, nullptr);

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}

	{ // Do output 
		std::ofstream fout(out_file);

		fout << "#define COMPILING_META_TYPES" << std::endl
			 << "#include \"" << in_file << "\"" << std::endl << std::endl
			 << "void make_meta_info() {  " << std::endl;

		print_basic_types(fout);
		print_results(fout);

		fout << "}" << std::endl << std::endl;
	}

	return 0;
}
