
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

template<typename A, typename M, typename T>
struct vec_printer {
    static u32 write(astring<A> out, u32 idx, T val) {
        u32 start = idx, i = 0;
        idx += out.write(idx, '[');
        for(auto& item : val) {
            idx += format_type<A, M, Type_Info<M>::type>::write(out, idx, item);
            if(i++ != val.size - 1) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(T val) {
        u32 idx = 2, i = 0;
        for(auto& item : val) {
            idx += format_type<A, M, Type_Info<M>::type>::size(item);
            if(i++ != val.size - 1) idx += 2;
        }
        return idx;
    }
};

template<typename A, typename M, typename VA>
struct format_type<A, vec<M,VA>, Type_Type::record_> {

    static u32 write(astring<A> out, u32 idx, vec<M,VA> val) {
        return vec_printer<A,M,vec<M,VA>>::write(out, idx, val);
    }
    static u32 size(vec<M,VA> val) {
        return vec_printer<A,M,vec<M,VA>>::size(val);
    }
};

template<typename A, typename M>
struct format_type<A, vec_view<M>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, vec_view<M> val) {
        return vec_printer<A,M,vec_view<M>>::write(out, idx, val);
    }
    static u32 size(vec_view<M> val) {
        return vec_printer<A,M,vec_view<M>>::size(val);
    }
};

template<typename A, typename M>
struct format_type<A, v_vec<M>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, v_vec<M> val) {
        return vec_printer<A,M,v_vec<M>>::write(out, idx, val);
    }
    static u32 size(v_vec<M> val) {
        return vec_printer<A,M,v_vec<M>>::size(val);
    }
};

template<typename A, typename M, typename HA>
struct format_type<A, heap<M,HA>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, heap<M,HA> val) {
        u32 start = idx, i = 0;
        idx += out.write(idx, "heap[");
        for(auto& item : val) {
            idx += format_type<A, M, Type_Info<M>::type>::write(out, idx, item);
            if(i++ != val.size - 1) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(heap<M,HA> val) {
        u32 idx = 6, i = 0;
        for(auto& item : val) {
            idx += format_type<A, M, Type_Info<M>::type>::size(item);
            if(i++ != val.size - 1) idx += 2;
        }
        return idx;
    }
};

template<typename A, typename K, typename V, typename MA, Hash<K> H>
struct format_type<A, map<K,V,MA,H>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, map<K,V,MA,H> val) {
        u32 start = idx, i = 0;
        idx += out.write(idx, '[');
        for(auto& item : val) {
            idx += out.write(idx, '{');
            idx += format_type<A, K, Type_Info<K>::type>::write(out, idx, item.key);
            idx += out.write(idx, " : ");
            idx += format_type<A, V, Type_Info<V>::type>::write(out, idx, item.value);
            idx += out.write(idx, '}');
            if(i++ != val.size - 1) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(map<K,V,MA,H> val) {
        u32 idx = 2, i = 0;
        for(auto& item : val) {
            idx += 5;
            idx += format_type<A, K, Type_Info<K>::type>::size(item.key);
            idx += format_type<A, V, Type_Info<V>::type>::size(item.value);
            if(i++ != val.size - 1) idx += 2;
        }
        return idx;
    }
};

template<typename A, typename M, typename SA>
struct format_type<A, stack<M,SA>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, stack<M,SA> val) {
        u32 start = idx;
        idx += out.write(idx, "stack");
        idx += format_type<A, vec<M,SA>, Type_Info<vec<M,SA>>::type>::write(out, idx, val.data);
        return idx - start;
    }
    static u32 size(stack<M,SA> val) {
        u32 idx = 5;
        idx += format_type<A, vec<M,SA>, Type_Info<vec<M,SA>>::type>::size(val.data);
        return idx;
    }
};

template<typename A, typename M, typename QA>
struct format_type<A, queue<M,QA>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, queue<M,QA> val) {
        u32 start = idx, i = 0;
        idx += out.write(idx, "queue[");
        for(auto& item : val) {
            idx += format_type<A, M, Type_Info<M>::type>::write(out, idx, item);
            if(i++ != val.size - 1) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(queue<M,QA> val) {
        u32 idx = 7, i = 0;
        for(auto& item : val) {
            idx += format_type<A, M, Type_Info<M>::type>::size(item);
            if(i++ != val.size - 1) idx += 2;
        }
        return idx;
    }
};

template<typename A, typename T, usize N>
struct format_type<A, vect<T,N>, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, vect<T,N> val) {
        u32 start = idx, i = 0;
        idx += out.write(idx, "[");
        for(auto& item : val) {
            idx += format_type<A, T, Type_Info<T>::type>::write(out, idx, item);
            if(i++ != N - 1) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(vect<T,N> val) {
        u32 idx = 2, i = 0;
        for(auto& item : val) {
            idx += format_type<A, T, Type_Info<T>::type>::size(item);
            if(i++ != N - 1) idx += 2;
        }
        return idx;
    }
};

template<typename A>
struct format_type<A, m4, Type_Type::record_> {
    static u32 write(astring<A> out, u32 idx, m4 val) {
        u32 start = idx, i = 0;
        idx += out.write(idx, "[");
        for(auto& item : val) {
            idx += format_type<A, v4, Type_Info<v4>::type>::write(out, idx, item);
            if(i++ != 3) idx += out.write(idx, ", ");
        }
        idx += out.write(idx, ']');
        return idx - start;
    }
    static u32 size(m4 val) {
        u32 idx = 2, i = 0;
        for(auto& item : val) {
            idx += format_type<A, v4, Type_Info<v4>::type>::size(item);
            if(i++ != 3) idx += 2;
        }
        return idx;
    }
};

// TODO(max): math types

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

        if (used == fmt.len - 1) {
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

        if (used == fmt.len - 1) {
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
string format(literal fmt, Ts... args) {

    u32 len = sprint_size<Ts...>(fmt, 0, args...);
    
    string ret = string::make(len + 1);
    ret.len = len + 1;

    u32 written = sprint<Mdefault,Ts...>(ret, fmt, 0, args...);
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