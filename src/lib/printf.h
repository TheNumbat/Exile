
#pragma once

#include "reflect.h"

template<typename... Ts>
string scratch_format(string fmt, Ts... args);

template<typename E, Type_Type T>
struct format_type {};

template<>
struct format_type<void, Type_Type::void_> {
    static u32 write(string out, u32 idx) {
        out.write(idx, "void");
    }
    static u32 size() {
        return 4;
    }
};

#pragma warning(push)
#pragma warning(disable : 4477)

template<typename E>
struct format_type<E, Type_Type::int_> {
    static u32 write(string out, u32 idx, E val) {
        if(Type_Info<E>::sgn) {
            switch(Type_Info<E>::size) {
            case 1: return sprintf(out.c_str + idx, "%hhd", val);
            case 2: return sprintf(out.c_str + idx, "%hd", val);
            case 4: return sprintf(out.c_str + idx, "%d", val);
            case 8: return sprintf(out.c_str + idx, "%lld", val);
            }
        } else {
            switch(Type_Info<E>::size) {
            case 1: return sprintf(out.c_str + idx, "%hhu", val);
            case 2: return sprintf(out.c_str + idx, "%hu", val);
            case 4: return sprintf(out.c_str + idx, "%u", val);
            case 8: return sprintf(out.c_str + idx, "%llu", val);
            }
        }
        return 0;
    }
    static u32 size(E val) {
        if(Type_Info<E>::sgn) {
            switch(Type_Info<E>::size) {
            case 1: return snprintf(null, 0, "%hhd", val);
            case 2: return snprintf(null, 0, "%hd", val);
            case 4: return snprintf(null, 0, "%d", val);
            case 8: return snprintf(null, 0, "%lld", val);
            }
        } else {
            switch(Type_Info<E>::size) {
            case 1: return snprintf(null, 0, "%hhu", val);
            case 2: return snprintf(null, 0, "%hu", val);
            case 4: return snprintf(null, 0, "%u", val);
            case 8: return snprintf(null, 0, "%llu", val);
            }
        }
        return 0;
    }
};

#pragma warning(pop)

template<typename E>
struct format_type<E, Type_Type::float_> {
    static u32 write(string out, u32 idx, E val) {
        return sprintf(out.c_str + idx, "%f", val);
    }
    static u32 size(E val) {
        return snprintf(null, 0, "%f", val);
    }
};

template<typename E>
struct format_type<E, Type_Type::bool_> {
    static u32 write(string out, u32 idx, E val) {
        return val ? out.write(idx, "true") : out.write(idx, "false");
    }
    static u32 size(E val) {
        return val ? 4 : 5;
    }
};

template<typename E>
struct format_type<E, Type_Type::string_> {
    static u32 write(string out, u32 idx, E val) {
        return out.write(idx, val);
    }
    static u32 size(E val) {
        return val.len;
    }
};

template<typename E>
struct format_type<E, Type_Type::array_> {
    using underlying = typename Type_Info<E>::underlying;
    using format_underlying = format_type<underlying, Type_Info<underlying>::type>;

    static u32 write(string out, u32 idx, E val) {
        u32 start = idx;
        idx += out.write(idx, "[");
        
        for(usize i = 0; i < Type_Info<E>::len; i++) {
            idx += format_underlying::write(out, idx, val[i]);
            if(i != Type_Info<E>::len - 1) {
                idx += out.write(idx, ", ");
            }
        }

        idx += out.write(idx, "]");
        return idx - start;
    }
    static u32 size(E val) {
        u32 idx = 1;
        for(usize i = 0; i < Type_Info<E>::len; i++) {
            idx += format_underlying::size(val[i]);
            if(i != Type_Info<E>::len - 1) {
                idx += 2;
            }
        }
        return idx + 1;
    }
};

template<typename... Ts>
u32 sprint(string out, string fmt, u32 idx) {
    return out.write(idx, fmt);
}
template<typename T, typename... Ts>
u32 sprint(string out, string fmt, u32 idx, T first, Ts... args) {

    u32 start = idx;
    u32 used = 0;

    while(fmt[used] != '%') {
        idx += out.write(idx, fmt[used]);
        used += 1;

        if(idx == fmt.len) {
            warn("Too few format specifiers in format string!");
            return idx - start;
        }
    }

    idx += format_type<T, Type_Info<T>::type>::write(out, idx, first);

	return idx - start + sprint(out, fmt.sub_end(used + 1), idx, args...);
}

template<typename... Ts>
u32 sprint_size(string fmt, u32 idx) {
    return fmt.len - 1;
}
template<typename T, typename... Ts>
u32 sprint_size(string fmt, u32 idx, T first, Ts... args) {

    u32 start = idx;
    u32 used = 0;

    while(fmt[used] != '%') {
        idx += 1;
        used += 1;

        if(idx == fmt.len) {
            warn("Too few format specifiers in format string!");
            return idx - start;
        }
    }

    idx += format_type<T, Type_Info<T>::type>::size(first);

	return idx - start + sprint_size(fmt.sub_end(used + 1), idx, args...);
}

template<typename... Ts>
string format(string fmt, Ts... args) {

    u32 len = sprint_size<Ts...>(fmt, 0, args...);
    
    string ret = string::make(len + 1);
    ret.len = len + 1;

    u32 written = sprint<Ts...>(ret, fmt, 0, args...);
    assert(len == written);
    return ret;
}

template<typename... Ts>
string scratch_format(string fmt, Ts... args) {

    u32 len = sprint_size<Ts...>(fmt, 0, args...);
    assert(len < g_scratch_buf.cap);
    g_scratch_buf.len = len + 1;

    sprint<Ts...>(g_scratch_buf, fmt, 0, args...);
    return g_scratch_buf;
}
