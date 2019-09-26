
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
    fptr_,
    enum_,
    record_
};

template<usize V, const char* N>
struct Enum_Field {
    static constexpr char const* name = N;
    static constexpr usize val = V;
};

template<typename T, usize O, const char* N>
struct Record_Field {
    static constexpr char const* name = N;
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
template<>
struct Type_List<void> {
    using head = void;
    using tail = void;
};

template<typename T> struct Type_Info;

template<typename T> 
struct Type_Info<T*> {
    using to = T;
    decltype(Type_Info<to>::name) name = Type_Info<to>::name;
    static constexpr usize size = sizeof(T*);
    static constexpr Type_Type type = Type_Type::ptr_;
};

#include "../build/meta_types.h"
