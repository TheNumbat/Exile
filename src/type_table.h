
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
	string 		signature;
	type_id 	return_type			= 0;
	type_id 	param_types[32] 	= {};
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
	i64			member_values[128] 	= {};
	u32 		member_count		= 0;
};
struct Type_string_info {};

// TODO(max): reduce memory footprint; Type_enum_info takes up way too much space
// 			  currently 3128 bytes / _type_info, type_table size is at least 3MB

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
#define TYPEINFO_H(h) (h ? type_table.try_get(h) : 0)
template<typename T>
struct _get_type_info { 
	static _type_info* get_type_info() {
		return type_table.try_get((type_id)typeid(T).hash_code());
	}
};

template<typename T>
struct _get_type_info<T*> {
	static _type_info* get_type_info() {

		_type_info* info = type_table.try_get((type_id)typeid(T*).hash_code());
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
		return type_table.insert(ptr_t.hash, ptr_t, false);
	}
};

void make_meta_info();
void make_type_table(allocator* alloc) { PROF

	type_table = map<type_id,_type_info>::make(1024, alloc, &hash_u64);

	{
		_type_info void_t;
		void_t.type_type 		= Type::_void;
		void_t.size				= 0;
		void_t.name 			= string_literal("void");
		void_t._int.is_signed 	= true;
		void_t.hash = (type_id)typeid(void).hash_code();
		type_table.insert(void_t.hash, void_t, false);
	}

	{
		_type_info char_t;
		char_t.type_type 		= Type::_int;
		char_t.size				= sizeof(char);
		char_t.name 			= string_literal("char");
		char_t._int.is_signed 	= true;
		char_t.hash = (type_id)typeid(char).hash_code();
		type_table.insert(char_t.hash, char_t, false);
	}

	{
		_type_info u8_t;
		u8_t.type_type 		= Type::_int;
		u8_t.size			= sizeof(u8);
		u8_t.name 			= string_literal("u8");
		u8_t._int.is_signed = false;
		u8_t.hash = (type_id)typeid(u8).hash_code();
		type_table.insert(u8_t.hash, u8_t, false);
	}

	{
		_type_info i8_t;
		i8_t.type_type 		= Type::_int;
		i8_t.size			= sizeof(i8);
		i8_t.name 			= string_literal("i8");
		i8_t._int.is_signed = true;
		i8_t.hash = (type_id)typeid(i8).hash_code();
		type_table.insert(i8_t.hash, i8_t, false);
	}

	{
		_type_info u16_t;
		u16_t.type_type 		= Type::_int;
		u16_t.size				= sizeof(u16);
		u16_t.name 				= string_literal("u16");
		u16_t._int.is_signed 	= false;
		u16_t.hash = (type_id)typeid(u16).hash_code();
		type_table.insert(u16_t.hash, u16_t, false);
	}

	{
		_type_info i16_t;
		i16_t.type_type 		= Type::_int;
		i16_t.size				= sizeof(i16);
		i16_t.name 				= string_literal("i16");
		i16_t._int.is_signed 	= true;
		i16_t.hash = (type_id)typeid(i16).hash_code();
		type_table.insert(i16_t.hash, i16_t, false);
	}

	{
		_type_info u32_t;
		u32_t.type_type 		= Type::_int;
		u32_t.size				= sizeof(u32);
		u32_t.name 				= string_literal("u32");
		u32_t._int.is_signed 	= false;
		u32_t.hash = (type_id)typeid(u32).hash_code();
		type_table.insert(u32_t.hash, u32_t, false);
	}

	{
		_type_info i32_t;
		i32_t.type_type 		= Type::_int;
		i32_t.size				= sizeof(i32);
		i32_t.name 				= string_literal("i32");
		i32_t._int.is_signed 	= true;
		i32_t.hash = (type_id)typeid(i32).hash_code();
		type_table.insert(i32_t.hash, i32_t, false);
	}

	{
		_type_info u64_t;
		u64_t.type_type 		= Type::_int;
		u64_t.size				= sizeof(u64);
		u64_t.name 				= string_literal("u64");
		u64_t._int.is_signed 	= false;
		u64_t.hash = (type_id)typeid(u64).hash_code();
		type_table.insert(u64_t.hash, u64_t, false);
	}

	{
		_type_info i64_t;
		i64_t.type_type 		= Type::_int;
		i64_t.size				= sizeof(i64);
		i64_t.name 				= string_literal("i64");
		i64_t._int.is_signed 	= true;
		i64_t.hash = (type_id)typeid(i64).hash_code();
		type_table.insert(i64_t.hash, i64_t, false);
	}

	{
		_type_info f32_t;
		f32_t.type_type 		= Type::_float;
		f32_t.size				= sizeof(f32);
		f32_t.name 				= string_literal("f32");
		f32_t._int.is_signed 	= true;
		f32_t.hash = (type_id)typeid(f32).hash_code();
		type_table.insert(f32_t.hash, f32_t, false);
	}

	{
		_type_info f64_t;
		f64_t.type_type 		= Type::_float;
		f64_t.size				= sizeof(f64);
		f64_t.name 				= string_literal("f64");
		f64_t._int.is_signed 	= true;
		f64_t.hash = (type_id)typeid(f64).hash_code();
		type_table.insert(f64_t.hash, f64_t, false);
	}

	{
		_type_info bool_t;
		bool_t.type_type 		= Type::_bool;
		bool_t.size				= sizeof(bool);
		bool_t.name 			= string_literal("bool");
		bool_t._int.is_signed 	= true;
		bool_t.hash = (type_id)typeid(bool).hash_code();
		type_table.insert(bool_t.hash, bool_t, false);
	}

	{
		_type_info string_t;
		string_t.type_type 		= Type::_string;
		string_t.size			= sizeof(string);
		string_t.name 			= string_literal("string");
		string_t.hash = (type_id)typeid(string).hash_code();
		type_table.insert(string_t.hash, string_t, false);
	}

	make_meta_info();
}

i64 int_as_i64(void* val, _type_info* info) { PROF
	switch(info->size) {
	case 1: {
		if(info->_int.is_signed) {
			return (i64)*(i8*)val;
		} else {
			return (i64)*(u8*)val;
		}
	} break;
	case 2: {
		if(info->_int.is_signed) {
			return (i64)*(i16*)val;
		} else {
			return (i64)*(u16*)val;
		}
	} break;
	case 4: {
		if(info->_int.is_signed) {
			return (i64)*(i32*)val;
		} else {
			return (i64)*(u32*)val;
		}
	} break;
	case 8: {
		if(info->_int.is_signed) {
			return (i64)*(i64*)val;
		} else {
			return (i64)*(u64*)val;
		}
	} break;
	default: {
		LOG_DEBUG_ASSERT(!"Int was not 1, 2, 4, or 8 bytes?!?!");
		return 0;
	} break;
	}
}
