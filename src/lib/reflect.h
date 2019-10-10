
#pragma once

/*
    NOTE(max):

    Technically the reflection system can work with literally anything, but the current meta-program
    doesn't handle nested template instantiation/defaulted template parameters/etc. very well. Thanks libclang! 
    I don't want to _again_ re-write the meta-program (this time as a true clang-tool) to be truly robust, 
    so instead just assume that no reflection code for template data structures will be generated. 

    99% of the templated structures we will want to reflect are just my standard library ones,
    so I will provide custom print/UI/serial/etc. code for all of the standard data structures.

    NOTE(max): I still can't get constexpr offset_of to work. The compiler doesn't accept it as a compile
    time constant, even if it literally confirms reduction to a literal. Maybe once we switch to clang...
*/

enum class Type_Type : u8 {
    void_,
    char_,
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

template<typename T>
struct No_Const {
    using type = T;
};

template<typename T>
struct No_Const<const T> {
    using type = T;
};

template<typename T, typename U> 
constexpr size_t offset_of(U T::*member) {
    return (char*)&((T*)null->*member) - (char*)null;
}

template<typename T> struct Type_Info;

template<typename T> 
struct Type_Info<T*> {
    using to = typename No_Const<T>::type;
    decltype(Type_Info<to>::name) name = Type_Info<to>::name;
    static constexpr usize size = sizeof(typename No_Const<T>::type*);
    static constexpr Type_Type type = Type_Type::ptr_;
};
