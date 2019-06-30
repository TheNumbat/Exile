
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <algorithm>

#include <clang-c/Index.h>

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

struct type_data {
	bool output = false;
	std::vector<CXCursor> references;
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
std::ofstream log_out, fout;

void print_data_type(CXType type, type_data& data);

CXChildVisitResult traversal(CXCursor c, CXCursor parent, CXClientData client_data) {

	if(clang_Location_isInSystemHeader(clang_getCursorLocation(c)) || 
	   clang_isPreprocessing(c.kind) || clang_isUnexposed(c.kind)  || 
	   clang_isInvalid(c.kind)) {

		return CXChildVisit_Continue;
	}

	if(c.kind == CXCursor_VarDecl || c.kind == CXCursor_ParmDecl ||
	   c.kind == CXCursor_CStyleCastExpr || c.kind == CXCursor_MemberRefExpr) {

		type_data data;
		data.references.push_back(c);
		types.insert({clang_getCursorType(c), data});
	}

	return CXChildVisit_Recurse;
}

CXChildVisitResult parse_enum(CXCursor c, CXCursor parent, CXClientData client_data) {

	int* i = (int*)client_data;

	if(c.kind == CXCursor_EnumConstantDecl) {

		uint64_t value = clang_getEnumConstantDeclUnsignedValue(c);
		std::string name = to_string(clang_getCursorSpelling(c));

		fout << "\t\tthis_type_info._enum.member_names[" << *i << "] = \"" << name << "\"_;" << std::endl
			 << "\t\tthis_type_info._enum.member_values[" << *i << "] = " << value << ";" << std::endl;

		(*i)++;
	}

	return CXChildVisit_Continue;
}

void print_enum(CXType type) {

	CXCursor decl = clang_getTypeDeclaration(type);
	CXType underlying = clang_getEnumDeclIntegerType(decl);
	
	std::string name = to_string(clang_getCursorSpelling(decl));
	std::string type_name = to_string(clang_getTypeSpelling(type));
	std::string underlying_name = to_string(clang_getTypeSpelling(underlying));

	fout << "\t[]() -> void {" << std::endl
		 << "\t\t_type_info this_type_info;" << std::endl
		 << "\t\tthis_type_info.type_type = Type::_enum;" << std::endl
		 << "\t\tthis_type_info.size = sizeof(" << type_name << ");" << std::endl
		 << "\t\tthis_type_info.name = \"" << name << "\"_;" << std::endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << type_name << ").hash_code();" << std::endl
		 << "\t\tthis_type_info._enum.base_type = TYPEINFO(" << underlying_name << ") ? TYPEINFO(" << underlying_name << ")->hash : 0;" << std::endl;

	int i = 0;

	clang_visitChildren(decl, parse_enum, &i);
	
	fout << "\t\tthis_type_info._enum.member_count = " << i << ";" << std::endl
		 << "\t\ttype_table.insert(this_type_info.hash, this_type_info, false);" << std::endl
		 << "\t}();" << std::endl << std::endl;		 
}

struct field_client {
	int i = 0;
	std::string outer;
};

CXVisitorResult print_field(CXCursor c, CXClientData client_data) {

	field_client* data = (field_client*)client_data;

	CXType type = clang_getCursorType(c);
	CXCursor decl = clang_getTypeDeclaration(type);

	if(clang_Cursor_isAnonymous(decl) && type.kind == CXType_Record) {
		clang_Type_visitFields(type, print_field, client_data);
		return CXVisit_Continue;
	}

	std::string name  = to_string(clang_getCursorSpelling(c));
	std::string type_name = to_string(clang_getTypeSpelling(type));

	fout << "\t\tthis_type_info._struct.member_types[" << data->i << "] = TYPEINFO(" << type_name << ") ? TYPEINFO(" << type_name << ")->hash : 0;" << std::endl
		 << "\t\tthis_type_info._struct.member_names[" << data->i << "] = \"" << name << "\"_;" << std::endl
		 << "\t\tthis_type_info._struct.member_offsets[" << data->i << "] = offsetof(" << data->outer << ", " << name << ");" << std::endl;

	data->i++;

	return CXVisit_Continue;
}

void print_record(CXType type) {

	CXCursor decl = clang_getTypeDeclaration(type);
	
	std::string name = to_string(clang_getCursorSpelling(decl));
	if(name == "string") return;

	std::string type_name = to_string(clang_getTypeSpelling(type));

	fout << "\t[]() -> void {" << std::endl
		 << "\t\t_type_info this_type_info;" << std::endl
		 << "\t\tthis_type_info.type_type = Type::_struct;" << std::endl
		 << "\t\tthis_type_info.size = sizeof(" << type_name << ");" << std::endl
		 << "\t\tthis_type_info.name = \"" << name << "\"_;" << std::endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << type_name << ").hash_code();" << std::endl;

	field_client data;
	data.outer = type_name;

	clang_Type_visitFields(type, print_field, &data);
	
	fout << "\t\tthis_type_info._struct.member_count = " << data.i << ";" << std::endl
		 << "\t\ttype_table.insert(this_type_info.hash, this_type_info, false);" << std::endl
		 << "\t}();" << std::endl << std::endl;
}

void print_array(CXType type) {

	CXCursor decl = clang_getTypeDeclaration(type);

	CXType underlying = clang_getArrayElementType(type);
	int64_t length = clang_getArraySize(type);

	auto entry = types.find(underlying);
	if(entry != types.end()) {
		print_data_type(entry->first, entry->second);
	}

	std::string type_name = to_string(clang_getTypeSpelling(type));
	std::string base_type_name = to_string(clang_getTypeSpelling(underlying));

	fout << "\t[]() -> void {" << std::endl
		 << "\t\t_type_info this_type_info;" << std::endl
		 << "\t\tthis_type_info.type_type = Type::_array;" << std::endl
		 << "\t\tthis_type_info.size = sizeof(" << type_name << ");" << std::endl
		 << "\t\tthis_type_info.name = \"" << type_name << "\"_;" << std::endl
		 << "\t\tthis_type_info.hash = (type_id)typeid(" << type_name << ").hash_code();" << std::endl
		 << "\t\tthis_type_info._array.of = TYPEINFO(" << base_type_name << ") ? TYPEINFO(" << base_type_name << ")->hash : 0;" << std::endl
		 << "\t\tthis_type_info._array.length = " << length << ";" << std::endl
		 << "\t\ttype_table.insert_if_unique(this_type_info.hash, this_type_info, false);" << std::endl
		 << "\t}();" << std::endl << std::endl;
}

void print_data_type(CXType type, type_data& data) {

	if(data.output) {
		log_out << "Already seen type: " << clang_getTypeSpelling(type) << std::endl;
		return;
	}

	log_out << "Type: " << clang_getTypeSpelling(type) << std::endl
			<< "references: ";
	for(CXCursor c : data.references) {
		CXSourceLocation loc = clang_getCursorLocation(c);
		CXFile file; unsigned int line;
		clang_getExpansionLocation(loc, &file, &line, nullptr, nullptr);
		log_out << clang_getFileName(file) << ":" << line << ", ";
	}
	log_out << std::endl;

	switch(type.kind) {
	
	case CXType_Enum: print_enum(type); break;
	case CXType_Record: print_record(type); break;
	case CXType_ConstantArray: print_array(type); break;

	case CXType_Elaborated: {
		CXType named = clang_Type_getNamedType(type);
		auto entry = types.find(named);
		if(entry != types.end()) {
			print_data_type(entry->first, entry->second);
		} else {
			type_data named_data;
			named_data.references.insert(named_data.references.begin(), data.references.begin(), data.references.end());
			print_data_type(named, named_data);
			types.insert({named, named_data});
		}
	} break;

	default: {
		log_out << "\tkind: " << clang_getTypeKindSpelling(type.kind) << std::endl; 
	} break;
	}

	data.output = true;
}

void print_results() {

	for(auto& entry : types) {
		print_data_type(entry.first, entry.second);
	}
}

void print_basic_types() {

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
			fout << "#define COMPILING_META_TYPES" << std::endl
				 << "#include \"" << in_file << "\"" << std::endl << std::endl
				 << "void make_meta_info() {  " << std::endl;

			print_basic_types();
			print_results();

			fout << "}" << std::endl << std::endl;
		}

		clang_disposeTranslationUnit(unit);
		clang_disposeIndex(index);
	}

	return 0;
}
