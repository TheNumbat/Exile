
#pragma once

extern std::atomic<i64> vallocs;

template<const char* tname>
template<typename T>
T* Mallocator<tname>::alloc(usize size, usize align) {
    T* ret = (T*)base_alloc(size);
    assert(ret);
    return ret;
}

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

template<const char* tname, usize N>
u8* MVarena<tname, N>::init() {
    mem = Mvirtual::alloc<u8>(N, 0);
    return mem;
}
template<const char* tname, usize N>
void MVarena<tname, N>::reset() {
    Mvirtual::dealloc(mem);
    mem = null;
    used = 0;
}

template<typename T>
T* Mvirtual::alloc(usize size, usize align) {
    vallocs++;
#ifdef _WIN32
    T* ret = (T*)VirtualAlloc(null, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    assert(ret);
#elif defined(__linux__) 
    T* ret = (T*)mmap(null, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    assert(ret != MAP_FAILED);
#endif
    return ret;
}

template<typename T>
void Mvirtual::dealloc(T* mem) {
    if(!mem) return;
    vallocs--;
#ifdef _WIN32
    VirtualFree(mem, 0, MEM_RELEASE);
#elif defined(__linux__) 
    munmap(mem, 0);
#endif
}
