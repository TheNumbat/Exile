
#pragma once

template<typename A>
char astring<A>::operator[](u32 idx) const {
    assert(idx < len);
    return c_str[idx];
}

template<typename A>
char& astring<A>::operator[](u32 idx) {
    assert(idx < len);
    return c_str[idx];
}

template<typename A>
const astring<A> astring<A>::sub_end(u32 s) const {
    assert(s <= len);
    astring ret;
    ret.c_str = c_str + s;
    ret.cap = cap - s;
    ret.len = len - s;
    return ret;
}

template<typename A>
u32 astring<A>::write(u32 idx, astring cpy) {
    assert(cap && idx + cpy.len - 1 < len);
    memcpy(c_str + idx, cpy.c_str, cpy.len - 1);
    return cpy.len - 1;
}
template<typename A>
u32 astring<A>::write(u32 idx, char cpy) {
    assert(cap && idx < len);
    c_str[idx] = cpy;
    return 1;
}

string last_file(string path) {
    u32 loc = path.len - 1;
    for(; loc >= 0; loc--) {
        if(path.c_str[loc] == '\\' || path.c_str[loc] == '/') {
            break;
        }
    }
    return path.sub_end(loc + 1);
}
