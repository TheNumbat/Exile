
#pragma once

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