
#include <typeinfo>

enum class Type : u8 {
	_unkown,

	_void,
	_int,
	_float,
	_bool,
	_ptr,		// generated on request (need to already have info of pointed-to type)
	_func,		// unimplemented (function pointer)
	_array,		// unimplemented (static array)
	_struct,
	_enum,		// only name implemented

	_string,
};

struct _type_info;
typedef u64 type_id;

struct Type_void_info {};
struct Type_int_info {
	bool is_signed = false;
};
struct Type_float_info {};
struct Type_bool_info {};
struct Type_ptr_info {
	type_id to = 0;
};
struct Type_func_info {
	type_id 	param_types[16] 	= {};
	string 		param_names[16];
	u32 		param_count 		= 0;
};
struct Type_array_info {
	type_id of 		= 0;
	u64 	length 	= 0;
};
struct Type_struct_info {
	type_id		member_types[64]	= {};
	string 		member_names[64];
	u32 		member_offsets[64]	= {};
	u32 		member_count		= 0;
};
struct Type_enum_info {
	type_id 	base_type			= 0;
	string 		member_names[128];
	u64			member_values[128] 	= {};
	u32 		member_count		= 0;
};
struct Type_string_info {};

// TODO(max): reduce memory footprint; Type_enum_info takes up way too much space

struct _type_info {
	Type type_type 	= Type::_unkown;
	type_id hash 	= 0;
	u32 size 		= 0;
	string name;
	union {
		Type_void_info	 _void;
		Type_int_info    _int;
		Type_float_info  _float;
		Type_bool_info   _bool;
		Type_ptr_info    _ptr;
		Type_func_info   _func;
		Type_array_info	 _array;
		Type_struct_info _struct;
		Type_enum_info   _enum;
		Type_string_info _string;
	};
	_type_info() : _void(), _int(), _float(), _bool(), _ptr(), _func(), _struct(), _enum(), _string() {}
};

thread_local map<type_id,_type_info> type_table;

#define TYPEINFO(...) _get_type_info<__VA_ARGS__>::get_type_info()
#define TYPEINFO_H(h) (h ? map_try_get(&type_table, h) : 0)
template<typename T>
struct _get_type_info { 
	static _type_info* get_type_info() {
		return map_try_get(&type_table, (type_id)typeid(T).hash_code());
	}
};

template<typename T>
struct _get_type_info<T*> {
	static _type_info* get_type_info() {

		_type_info* info = map_try_get(&type_table, (type_id)typeid(T*).hash_code());
		if(info) return info;

		_type_info ptr_t;
		ptr_t.type_type = Type::_ptr;
		ptr_t.size 		= sizeof(T*);

		_type_info* to = TYPEINFO(T);

		if (to) {
			ptr_t.name = to->name;
			ptr_t._ptr.to = to->hash;
		}
		else {
			ptr_t.name = string_literal("UNDEF");
			ptr_t._ptr.to = 0;
		}
		
		ptr_t.hash = (type_id)typeid(T*).hash_code();
		return map_insert(&type_table, ptr_t.hash, ptr_t, false);
	}
};

void make_meta_types();
void make_type_table(allocator* alloc) {

	type_table = make_map<type_id,_type_info>(512, alloc, &hash_u64);

	{
		_type_info void_t;
		void_t.type_type 		= Type::_void;
		void_t.size				= 0;
		void_t.name 			= string_literal("void");
		void_t._int.is_signed 	= true;
		void_t.hash = (type_id)typeid(void).hash_code();
		map_insert(&type_table, void_t.hash, void_t, false);
	}

	{
		_type_info char_t;
		char_t.type_type 		= Type::_int;
		char_t.size				= sizeof(char);
		char_t.name 			= string_literal("char");
		char_t._int.is_signed 	= true;
		char_t.hash = (type_id)typeid(char).hash_code();
		map_insert(&type_table, char_t.hash, char_t, false);
	}

	{
		_type_info u8_t;
		u8_t.type_type 		= Type::_int;
		u8_t.size			= sizeof(u8);
		u8_t.name 			= string_literal("u8");
		u8_t._int.is_signed = false;
		u8_t.hash = (type_id)typeid(u8).hash_code();
		map_insert(&type_table, u8_t.hash, u8_t, false);
	}

	{
		_type_info i8_t;
		i8_t.type_type 		= Type::_int;
		i8_t.size			= sizeof(i8);
		i8_t.name 			= string_literal("i8");
		i8_t._int.is_signed = true;
		i8_t.hash = (type_id)typeid(i8).hash_code();
		map_insert(&type_table, i8_t.hash, i8_t, false);
	}

	{
		_type_info u16_t;
		u16_t.type_type 		= Type::_int;
		u16_t.size				= sizeof(u16);
		u16_t.name 				= string_literal("u16");
		u16_t._int.is_signed 	= false;
		u16_t.hash = (type_id)typeid(u16).hash_code();
		map_insert(&type_table, u16_t.hash, u16_t, false);
	}

	{
		_type_info i16_t;
		i16_t.type_type 		= Type::_int;
		i16_t.size				= sizeof(i16);
		i16_t.name 				= string_literal("i16");
		i16_t._int.is_signed 	= true;
		i16_t.hash = (type_id)typeid(i16).hash_code();
		map_insert(&type_table, i16_t.hash, i16_t, false);
	}

	{
		_type_info u32_t;
		u32_t.type_type 		= Type::_int;
		u32_t.size				= sizeof(u32);
		u32_t.name 				= string_literal("u32");
		u32_t._int.is_signed 	= false;
		u32_t.hash = (type_id)typeid(u32).hash_code();
		map_insert(&type_table, u32_t.hash, u32_t, false);
	}

	{
		_type_info i32_t;
		i32_t.type_type 		= Type::_int;
		i32_t.size				= sizeof(i32);
		i32_t.name 				= string_literal("i32");
		i32_t._int.is_signed 	= true;
		i32_t.hash = (type_id)typeid(i32).hash_code();
		map_insert(&type_table, i32_t.hash, i32_t, false);
	}

	{
		_type_info u64_t;
		u64_t.type_type 		= Type::_int;
		u64_t.size				= sizeof(u64);
		u64_t.name 				= string_literal("u64");
		u64_t._int.is_signed 	= false;
		u64_t.hash = (type_id)typeid(u64).hash_code();
		map_insert(&type_table, u64_t.hash, u64_t, false);
	}

	{
		_type_info i64_t;
		i64_t.type_type 		= Type::_int;
		i64_t.size				= sizeof(i64);
		i64_t.name 				= string_literal("i64");
		i64_t._int.is_signed 	= true;
		i64_t.hash = (type_id)typeid(i64).hash_code();
		map_insert(&type_table, i64_t.hash, i64_t, false);
	}

	{
		_type_info f32_t;
		f32_t.type_type 		= Type::_float;
		f32_t.size				= sizeof(f32);
		f32_t.name 				= string_literal("f32");
		f32_t._int.is_signed 	= true;
		f32_t.hash = (type_id)typeid(f32).hash_code();
		map_insert(&type_table, f32_t.hash, f32_t, false);
	}

	{
		_type_info f64_t;
		f64_t.type_type 		= Type::_float;
		f64_t.size				= sizeof(f64);
		f64_t.name 				= string_literal("f64");
		f64_t._int.is_signed 	= true;
		f64_t.hash = (type_id)typeid(f64).hash_code();
		map_insert(&type_table, f64_t.hash, f64_t, false);
	}

	{
		_type_info bool_t;
		bool_t.type_type 		= Type::_bool;
		bool_t.size				= sizeof(bool);
		bool_t.name 			= string_literal("bool");
		bool_t._int.is_signed 	= true;
		bool_t.hash = (type_id)typeid(bool).hash_code();
		map_insert(&type_table, bool_t.hash, bool_t, false);
	}

	{
		_type_info string_t;
		string_t.type_type 		= Type::_string;
		string_t.size			= sizeof(string);
		string_t.name 			= string_literal("string");
		string_t._int.is_signed = true;
		string_t.hash = (type_id)typeid(string).hash_code();
		map_insert(&type_table, string_t.hash, string_t, false);
	}

	make_meta_types();
}

void destroy_type_table() {
	destroy_map(&type_table);	
}

// we can ignore typedefs; they are simply aliases and (type_id)typeid() will be the same
