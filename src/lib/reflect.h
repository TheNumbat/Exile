
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
    array_
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

template<typename E, Type_Type T>
struct print_type {};

template<typename T> 
void print(T first);
template<typename T, typename... Ts> 
void print(T first, Ts... args);

template<>
struct print_type<void, Type_Type::void_> {
    static void print() {
        puts("void");
    }
};

template<typename E>
struct print_type<E, Type_Type::int_> {
    static void print(E val) {
        printf("%d", val);
    }
};

template<typename E>
struct print_type<E, Type_Type::float_> {
    static void print(E val) {
        printf("%f", val);
    }
};

template<typename E>
struct print_type<E, Type_Type::bool_> {
    static void print(E val) {
        val ? puts("true") : puts("false");
    }
};

template<typename E>
struct print_type<E, Type_Type::string_> {
    static void print(E val) {
        puts(val.c_str);
    }
};

template<typename E>
struct print_type<E, Type_Type::array_> {
    static void print(E val) {
        puts("[");
        for(usize i = 0; i < Type_Info<E>::len; i++) {
            ::print(val[i]);
            if(i != Type_Info<E>::len - 1) {
                puts(", ");
            }
        }
        puts("]");
    }
};

template<typename T>
void print(T first) {
    print_type<T, Type_Info<T>::type>::print(first);
}
template<typename T, typename... Ts>
void print(T first, Ts... args) {

    print_type<T, Type_Info<T>::type>::print(first);
    puts(", ");
	print(args...);
}


