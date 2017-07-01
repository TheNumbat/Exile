
#include <typeinfo>

enum class Type : u8 {
	unkown,

	_int,
	_float,
	_bool,
	_ptr,		// only when used in struct
	_func,		// only when used in struct
	_struct,
	_enum,

	_string,
	// vec,	these are templates...
	// map,	these are templates...
};

struct _type_info;

struct Type_int_info {
	bool is_signed;
};
struct Type_float_info {};
struct Type_bool_info {};
struct Type_ptr_info {
	_type_info* to;
};
struct Type_func_info {
	_type_info* param_types[16];
	string 		param_names[16];
	u32 		param_count;
};
struct Type_struct_info {
	_type_info* member_types[32];
	string 		member_names[32];
	u32 		member_offsets[32];
	u32 		member_count;
};
struct Type_enum_info {
	_type_info* base_type;
	string 		member_names[128];
	u64			member_values[128];
	u32 		member_count;
};
struct Type_string_info {};

// TODO(max): reduce memory footprint; Type_enum_info takes up way too much space

struct _type_info {
	Type type_type 	= Type::unkown;
	u32 size 				= 0;
	string name;
	union {
		Type_int_info    _int;
		Type_float_info  _float;
		Type_bool_info   _bool;
		Type_ptr_info    _ptr;
		Type_func_info   _func;
		Type_struct_info _struct;
		Type_enum_info   _enum;
		Type_string_info _string;
	};
	_type_info() : _int(), _float(), _bool(), _ptr(), _func(), _struct(), _enum(), _string() {}
};

static map<u64,_type_info> type_table;

void make_meta_types();

#define TYPEINFO(type) _get_type_info<type>().get_type_info(string_literal(#type));
template<typename T>
struct _get_type_info { 
	_type_info* get_type_info(string n) {
		return map_try_get(&type_table, typeid(T).hash_code());
	}
};

template<typename T>
struct _get_type_info<T*> {
	_type_info* get_type_info(string n) {

		_type_info* info = map_try_get(&type_table, typeid(T*).hash_code());
		if(info) return info;

		_type_info ptr_t;
		ptr_t.type_type = Type::_ptr;
		ptr_t.size 		= sizeof(T*);
		ptr_t.name 		= n;
		ptr_t._ptr.to 	= TYPEINFO(T);
		u64 hash = (u64)typeid(T*).hash_code();
		return map_insert(&type_table, hash, ptr_t, false);
	}
};

void make_type_table(allocator* alloc) {

	type_table = make_map<u64,_type_info>(1024, alloc, &hash_u64);

	{
		_type_info char_t;
		char_t.type_type 		= Type::_int;
		char_t.size				= sizeof(char);
		char_t.name 			= string_literal("char");
		char_t._int.is_signed 	= true;
		u64 hash = (u64)typeid(char).hash_code();
		map_insert(&type_table, hash, char_t, false);
	}

	{
		_type_info u8_t;
		u8_t.type_type 		= Type::_int;
		u8_t.size			= sizeof(u8);
		u8_t.name 			= string_literal("u8");
		u8_t._int.is_signed = false;
		u64 hash = (u64)typeid(u8).hash_code();
		map_insert(&type_table, hash, u8_t, false);
	}

	{
		_type_info i8_t;
		i8_t.type_type 		= Type::_int;
		i8_t.size			= sizeof(i8);
		i8_t.name 			= string_literal("i8");
		i8_t._int.is_signed = true;
		u64 hash = (u64)typeid(i8).hash_code();
		map_insert(&type_table, hash, i8_t, false);
	}

	{
		_type_info u16_t;
		u16_t.type_type 		= Type::_int;
		u16_t.size				= sizeof(u16);
		u16_t.name 				= string_literal("u16");
		u16_t._int.is_signed 	= false;
		u64 hash = (u64)typeid(u16).hash_code();
		map_insert(&type_table, hash, u16_t, false);
	}

	{
		_type_info i16_t;
		i16_t.type_type 		= Type::_int;
		i16_t.size				= sizeof(i16);
		i16_t.name 				= string_literal("i16");
		i16_t._int.is_signed 	= true;
		u64 hash = (u64)typeid(i16).hash_code();
		map_insert(&type_table, hash, i16_t, false);
	}

	{
		_type_info u32_t;
		u32_t.type_type 		= Type::_int;
		u32_t.size				= sizeof(u32);
		u32_t.name 				= string_literal("u32");
		u32_t._int.is_signed 	= false;
		u64 hash = (u64)typeid(u32).hash_code();
		map_insert(&type_table, hash, u32_t, false);
	}

	{
		_type_info i32_t;
		i32_t.type_type 		= Type::_int;
		i32_t.size				= sizeof(i32);
		i32_t.name 				= string_literal("i32");
		i32_t._int.is_signed 	= true;
		u64 hash = (u64)typeid(i32).hash_code();
		map_insert(&type_table, hash, i32_t, false);
	}

	{
		_type_info u64_t;
		u64_t.type_type 		= Type::_int;
		u64_t.size				= sizeof(u64);
		u64_t.name 				= string_literal("u64");
		u64_t._int.is_signed 	= false;
		u64 hash = (u64)typeid(u64).hash_code();
		map_insert(&type_table, hash, u64_t, false);
	}

	{
		_type_info i64_t;
		i64_t.type_type 		= Type::_int;
		i64_t.size				= sizeof(i64);
		i64_t.name 				= string_literal("i64");
		i64_t._int.is_signed 	= true;
		u64 hash = (u64)typeid(i64).hash_code();
		map_insert(&type_table, hash, i64_t, false);
	}

	{
		_type_info f32_t;
		f32_t.type_type 		= Type::_float;
		f32_t.size				= sizeof(f32);
		f32_t.name 				= string_literal("f32");
		f32_t._int.is_signed 	= true;
		u64 hash = (u64)typeid(f32).hash_code();
		map_insert(&type_table, hash, f32_t, false);
	}

	{
		_type_info f64_t;
		f64_t.type_type 		= Type::_float;
		f64_t.size				= sizeof(f64);
		f64_t.name 				= string_literal("f64");
		f64_t._int.is_signed 	= true;
		u64 hash = (u64)typeid(f64).hash_code();
		map_insert(&type_table, hash, f64_t, false);
	}

	{
		_type_info bool_t;
		bool_t.type_type 		= Type::_bool;
		bool_t.size				= sizeof(bool);
		bool_t.name 			= string_literal("bool");
		bool_t._int.is_signed 	= true;
		u64 hash = (u64)typeid(bool).hash_code();
		map_insert(&type_table, hash, bool_t, false);
	}

	{
		_type_info string_t;
		string_t.type_type 		= Type::_string;
		string_t.size			= sizeof(string);
		string_t.name 			= string_literal("string");
		string_t._int.is_signed = true;
		u64 hash = (u64)typeid(string).hash_code();
		map_insert(&type_table, hash, string_t, false);
	}

	make_meta_types();
}

void destroy_type_table() {
	destroy_map(&type_table);	
}

// we can ignore typedefs; they are simply aliases and (u64)typeid() will be the same
