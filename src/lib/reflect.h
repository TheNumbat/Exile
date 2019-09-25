
#pragma once

#include "basic.h"
#include "string.h"

typedef u64 type_id;

enum class Type : u8 {
    void_ = 0,
	int_,
	float_,
	bool_,
	string_,
	ptr_,
	func_,
	array_,
	struct_,
	enum_
};

struct Type_Void {};
struct Type_Int {
	bool is_signed = false;
};
struct Type_Float {};
struct Type_Bool {};
struct Type_Ptr {
	type_id to = 0;
};
struct Type_Func {
	string 		signature;
	type_id 	return_type			= 0;
	type_id 	param_types[16] 	= {};
	u32 		param_count 		= 0;
};
struct Type_Array {
	type_id of 		= 0;
	u64 	length 	= 0;
};
struct Type_Struct {
	type_id		member_types[96]	= {};
	string 		member_names[96];
	u32 		member_offsets[96]	= {};
	u32 		member_count		= 0;
};
struct Type_Enum {
	type_id 	base_type			= 0;
	string 		member_names[256];
	i64			member_values[256] 	= {};
	u32 		member_count		= 0;
};
struct Type_String {};

struct Type_Info {
	Type type	 = Type::void_;
	type_id hash = 0;
	u32 size 	 = 0;
	string name;
	union {
		Type_Void	void_;
		Type_Int    int_;
		Type_Float  float_;
		Type_Bool   bool_;
		Type_Ptr    ptr_;
		Type_Func 	func_;
		Type_Array	array_;
		Type_Struct struct_;
		Type_Enum   enum_;
		Type_String string_;
	};
};
