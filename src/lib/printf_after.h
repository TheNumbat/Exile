
#pragma once

template<typename A, typename SA>
struct format_type<A, astring<SA>, Type_Type::string_> {
    static u32 write(astring<A> out, u32 idx, astring<SA> val) {
        return out.write(idx, val);
    }
    static u32 size(astring<SA> val) {
        return val.len - 1;
    }
};

template<typename A, typename M, typename VA>
struct format_type<A, vec<M,VA>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, vec<M,VA> val) {
        u32 start = idx;
        idx += out.write(idx, '[');
        for(u32 i = 0; i < val.size; i++) {
            idx += format_type<M, Type_Info<M>::type>::write(out, idx, val[i]);
            if(i != val.size - 1) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(vec<M,VA> val) {
        u32 idx = 1;
        for(u32 i = 0; i < val.size; i++) {
            idx += format_type<M, Type_Info<M>::type>::size(val[i]);
            if(i != val.size - 1) idx += 2;
        }
        return idx + 1;
    }
};

template<typename A, typename M, typename VA>
struct format_type<A, stack<M,VA>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, stack<M,VA> val) {
        u32 start = idx;
        idx += out.write(idx, "stack[");
        for(u32 i = 0; i < val.data.size; i++) {
            idx += format_type<M, Type_Info<M>::type>::write(out, idx, val.data[i]);
            if(i != val.data.size - 1) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(stack<M,VA> val) {
        u32 idx = 6;
        for(u32 i = 0; i < val.data.size; i++) {
            idx += format_type<M, Type_Info<M>::type>::size(val.data[i]);
            if(i != val.data.size - 1) idx += 2;
        }
        return idx + 1;
    }
};

template<typename A, typename M, typename QA>
struct format_type<A, queue<M,QA>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, queue<M,QA> val) {
        u32 start = idx, i = 0;
        idx += out.write(idx, "queue[");
        for(auto item : val) {
            idx += format_type<M, Type_Info<M>::type>::write(out, idx, item);
            if(i++ != val.size - 1) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(queue<M,QA> val) {
        u32 idx = 6, i = 0;
        for(auto item : val) {
            idx += format_type<M, Type_Info<M>::type>::size(item);
            if(i++ != val.size - 1) idx += 2;
        }
        return idx + 1;
    }
};

template<typename A, typename T, typename... Ts>
u32 sprint(astring<A> out, literal fmt, u32 idx, T first, Ts... args) {

    u32 start = idx;
    u32 used = 0;

    while(true) {
        if (fmt[used] == '%') {
            if (used + 1 < fmt.len && fmt[used + 1] == '%') {
                used += 1;
            } else {
                break;
            }
        }

        idx += out.write(idx, fmt[used]);
        used += 1;

        if (used == fmt.len) {
            die("Too few format specifiers in format string!");
            return idx - start;
        }
    }

    idx += format_type<A, T, Type_Info<T>::type>::write(out, idx, first);

    return idx - start + sprint(out, fmt.sub_end(used + 1), idx, args...);
}

template<typename T, typename... Ts>
u32 sprint_size(literal fmt, u32 idx, T first, Ts... args) {

    u32 start = idx;
    u32 used = 0;

    while(true) {
        if (fmt[used] == '%') {
            if (used + 1 < fmt.len && fmt[used + 1] == '%') {
                used += 1;
            } else {
                break;
            }
        }

        idx += 1;
        used += 1;

        if (used == fmt.len) {
            die("Too few format specifiers in format string!");
            return idx - start;
        }
    }

    idx += format_type<void, T, Type_Info<T>::type>::size(first);

    return idx - start + sprint_size(fmt.sub_end(used + 1), idx, args...);
}

template<typename A, typename... Ts>
astring<A> format(literal fmt, Ts... args) {

    u32 len = sprint_size<Ts...>(fmt, 0, args...);
    
    astring<A> ret = astring<A>::make(len + 1);
    ret.len = len + 1;

    u32 written = sprint<Ts...>(ret, fmt, 0, args...);
    assert(len == written);
    return ret;
}

template<typename... Ts>
string scratch_format(literal fmt, Ts... args) {

    u32 len = sprint_size<Ts...>(fmt, 0, args...);
    
    assert(len < g_scratch_buf.cap);
    g_scratch_buf.len = len + 1;

    u32 written = sprint<Mdefault, Ts...>(g_scratch_buf, fmt, 0, args...);
    assert(len == written);

    g_scratch_buf[len] = 0;

    return g_scratch_buf;
}