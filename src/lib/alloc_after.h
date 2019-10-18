
#pragma once

template<const char* tname, usize N>
template<typename T>
T* MSarena<tname, N>::alloc(usize size, usize align) {
    uptr here = (uptr)mem + used;
    uptr offset = here % align;
    uptr next = here + (offset ? align - offset : 0);
    assert(next + size - (uptr)mem < N);
    T* ret = (T*)next;
    used += offset + size;
    high_water = _MAX(high_water, used);
    return ret;
}

template<const char* tname, usize N>
template<typename T>
T* MVarena<tname, N>::alloc(usize size, usize align) {
    if(!mem) init();
    uptr here = (uptr)mem + used;
    uptr offset = here % align;
    uptr next = here + (offset ? align - offset : 0);
    assert(next + size - (uptr)mem < N);
    T* ret = (T*)next;
    used += offset + size;
    high_water = _MAX(high_water, used);
    return ret;
}
