
enum class Type : u8 {
	_unkown,

	_void,
	_int,
	_float,
	_bool,
	_ptr,		// generated on request (need to already have info of pointed-to type)
	_func,		
	_array,		
	_struct,
	_enum,		

	_string,
};

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
	type_id 	param_types[16] 	= {};
	u32 		param_count 		= 0;
};
struct Type_array_info {
	type_id of 		= 0;
	u64 	length 	= 0;
};
struct Type_struct_info {
	type_id		member_types[96]	= {};
	string 		member_names[96];
	u32 		member_offsets[96]	= {};
	u8 			member_circular[96] = {};
	u32 		member_count		= 0;
};
struct Type_enum_info {
	type_id 	base_type			= 0;
	string 		member_names[256];				// NOTE(max): monkaS this is wayyy to big
	i64			member_values[256] 	= {};
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
	_type_info() {}
};

struct any {
	type_id id  = 0;
	void* value = null;

	_type_info* info();

	template<typename T> static any make(T* val);
};

thread_local map<type_id,_type_info> type_table;

#define TYPEINFO(...) _get_type_info< __VA_ARGS__ >::get_type_info()
#define TYPENAME(...) TYPEINFO(__VA_ARGS__)->name
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
			ptr_t.name = "UNDEF"_;
			ptr_t._ptr.to = 0;
		}
		
		ptr_t.hash = (type_id)typeid(T*).hash_code();
		return type_table.insert(ptr_t.hash, ptr_t, false);
	}
};

void make_meta_info();
void make_type_table(allocator* alloc);

template<typename E> E string_to_enum(string name);
template<typename E> string enum_to_string(E val);
string enum_to_string(i64 val, _type_info* info);

i64 int_as_i64(void* val, _type_info* info);
void int_from_i64(i64 val, void* i, _type_info* info);
f64 float_as_f64(void* val, _type_info* info);
