
#pragma once

#include "basic.h"
#include "string.h"
#include <stdio.h>

typedef u64 type_id;

enum class Type_Type : u8 {
    void_,
    int_,
    float_,
    bool_,
    string_,
    array_,
    ptr_,
};

template<typename T, typename... Ts>
struct Type_List {
    using head = T;
    using tail = Type_List<Ts...>;
};
template<typename T>
struct Type_List<T> {
    using head = T;
    using tail = void;
};

template<typename T> struct Type_Info;
#include "../build/meta_types.h"
