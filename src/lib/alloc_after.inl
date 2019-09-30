template<const char* tname, usize N>
template<typename T>
T* Marena<tname, N>::alloc(usize size, usize align) {
    uptr here = (uptr)mem + used;
    uptr offset = here % align;
    uptr next = here + (offset ? align - offset : 0);
    assert(next + size - (uptr)mem < N);
    T* ret = (T*)next;
    used = offset + size;
    return ret;
}