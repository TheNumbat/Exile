
#pragma once

#include "reflect.h"
#include <string.h>

template<typename... Ts>
string scratch_format(string fmt, Ts... args);

template<typename E, Type_Type T>
struct format_type {};

template<>
struct norefl format_type<void, Type_Type::void_> {
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
        return val.len - 1;
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

template<typename E>
struct format_type<E, Type_Type::ptr_> {
    using to = typename Type_Info<E>::to;
    using format_to = format_type<to, Type_Info<to>::type>;

    static u32 write(string out, u32 idx, E val) {
        u32 start = idx;
        idx += out.write(idx, '(');
        if(val) idx += format_to::write(out, idx, *val);
        else idx += out.write(idx, "null");
        idx += out.write(idx, ')');
        return idx - start;
    }
    static u32 size(E val) {
        if(val) return 2 + format_to::size(*val);
        else return 6;
    }
};

template<>
struct norefl format_type<decltype(nullptr), Type_Type::ptr_> {
    static u32 write(string out, u32 idx, decltype(nullptr) val) {
        return out.write(idx, "(null)");
    }
    static u32 size(decltype(nullptr) val) {
        return 6;
    }
};

template<typename E>
struct format_type<E, Type_Type::fptr_> {
    static u32 write(string out, u32 idx, E val) {
        u32 start = idx;
        idx += out.write(idx, '(');
        if(val) idx += out.write(idx, Type_Info<E>::name);
        else idx += out.write(idx, "null");
        idx += out.write(idx, ')');
        return idx - start;
    }
    static u32 size(E val) {
        if(val) return 2 + strlen(Type_Info<E>::name);
        else return 6;
    }
};

template<typename E, typename H, typename T>
struct format_member {
    static u32 write(string out, u32 idx, E val) {
        if((usize)val == H::val) return out.write(idx, H::name);
        else return format_member<E, typename T::head, typename T::tail>::write(out, idx, val);
    }
    static u32 size(E val) {
        if((usize)val == H::val) return strlen(H::name);
        else return format_member<E, typename T::head, typename T::tail>::size(val);
    }
};

template<typename E, typename H>
struct format_member<E, H, void> {
    static u32 write(string out, u32 idx, E val) {
        if((usize)val == H::val) return out.write(idx, H::name);
        else return out.write(idx, "NONE");
    }
    static u32 size(E val) {
        if((usize)val == H::val) return strlen(H::name);
        else return 4;
    }
};

template<typename E>
struct format_member<E, void, void> {
    static u32 write(string out, u32 idx, E val) {
        return 0;
    }
    static u32 size(E val) {
        return 0;
    }
};

template<typename E>
struct format_type<E, Type_Type::enum_> {
    using underlying = typename Type_Info<E>::underlying;
    using members = typename Type_Info<E>::members;

    static u32 write(string out, u32 idx, E val) {
        u32 start = idx;
        idx += out.write(idx, Type_Info<E>::name);
        idx += out.write(idx, "::");
        idx += format_member<E, typename members::head, typename members::tail>::write(out, idx, val);
        return idx - start;
    }
    static u32 size(E val) {
        u32 idx = 0;
        idx += strlen(Type_Info<E>::name);
        idx += 2;
        idx += format_member<E, typename members::head, typename members::tail>::size(val);
        return idx;
    }
};

template<typename E, typename H, typename T>
struct format_field {
    using member = typename H::type;
    static u32 write(string out, u32 idx, E val) {
        u32 start = idx;
        idx += out.write(idx, H::name);
        idx += out.write(idx, " : ");
        idx += format_type<member, Type_Info<member>::type>::write(out, idx, *(member*)((char*)&val + H::offset));
        idx += out.write(idx, ", ");
        idx += format_field<E, typename T::head, typename T::tail>::write(out, idx, val);
        return idx - start;
    }
    static u32 size(E val) {
        u32 idx = 0;
        idx += strlen(H::name);
        idx += 3;
        idx += format_type<member, Type_Info<member>::type>::size(*(member*)((char*)&val + H::offset));
        idx += 2;
        idx += format_field<E, typename T::head, typename T::tail>::size(val);
        return idx;
    }
};

template<typename E, typename H>
struct format_field<E, H, void> {
    using member = typename H::type;
    static u32 write(string out, u32 idx, E val) {
        u32 start = idx;
        idx += out.write(idx, H::name);
        idx += out.write(idx, " : ");
        idx += format_type<member, Type_Info<member>::type>::write(out, idx, *(member*)((char*)&val + H::offset));
        return idx - start;
    }
    static u32 size(E val) {
        u32 idx = 0;
        idx += strlen(H::name);
        idx += 3;
        idx += format_type<member, Type_Info<member>::type>::size(*(member*)((char*)&val + H::offset));
        return idx;
    }
};

template<typename E>
struct format_field<E, void, void> {
    static u32 write(string out, u32 idx) {
        return 0;
    }
    static u32 size() {
        return 0;
    }
};

template<typename E>
struct format_type<E, Type_Type::record_> {
    using members = typename Type_Info<E>::members;

    static u32 write(string out, u32 idx, E val) {
        u32 start = idx;
        idx += out.write(idx, Type_Info<E>::name);
        idx += out.write(idx, '{');
        idx += format_field<E, typename members::head, typename members::tail>::write(out, idx, val);
        idx += out.write(idx, '}');
        return idx - start;
    }
    static u32 size(E val) {
        u32 idx = 0;
        idx += strlen(Type_Info<E>::name);
        idx += 1;
        idx += format_field<E, typename members::head, typename members::tail>::size(val);
        return idx + 1;
    }
};

template<typename M>
struct format_type<vec<M>, Type_Type::record_> {
    static u32 write(string out, u32 idx, vec<M> val) {
        u32 start = idx;
        idx += out.write(idx, '[');
        for(u32 i = 0; i < val.size; i++) {
            idx += format_type<M, Type_Info<M>::type>::write(out, idx, val[i]);
            if(i != val.size - 1) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(vec<M> val) {
        u32 idx = 1;
        for(u32 i = 0; i < val.size; i++) {
            idx += format_type<M, Type_Info<M>::type>::size(val[i]);
            if(i != val.size - 1) idx += 2;
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

        if(used == fmt.len) {
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

        if(used == fmt.len) {
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

    u32 written = sprint<Ts...>(g_scratch_buf, fmt, 0, args...);
    assert(len == written);

    g_scratch_buf[len] = 0;

    return g_scratch_buf;
}
