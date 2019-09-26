
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
    enum_
};

template<usize V, const char* N>
struct Enum_Field {
    static constexpr char const* name = N;
    static constexpr usize val = V;
};

template<typename T, usize O, char... N>
struct Record_Field {
    static constexpr const char* name = N;
    static constexpr usize offset = O;
    using type = T;
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
