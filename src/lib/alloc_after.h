
#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#elif defined(__linux__)
    #include <sys/mman.h>
#endif

inline std::atomic<i64> allocs;
inline std::atomic<i64> vallocs;

template<const char* tname, bool log>
template<typename T>
T* Mallocator<tname,log>::alloc(usize size) {
    T* ret = (T*)base_alloc(size);
    if constexpr(log) Profiler::alloc({tname, ret, size});
    return ret;
}

template<const char* tname, bool log>
template<typename T>
void Mallocator<tname,log>::dealloc(T* mem) {
    if(!mem) return;
    if constexpr(log) Profiler::alloc({tname, mem, 0});
    base_free(mem);
}

template<const char* tname>
template<typename T>
T* Mvallocator<tname>::alloc(usize size) {
    T* ret = (T*)virt_alloc(size);
    Profiler::alloc({tname, ret, size});
    return ret;
}

template<const char* tname>
template<typename T>
void Mvallocator<tname>::dealloc(T* mem) {
    Profiler::alloc({tname, mem, 0});
    virt_free(mem);
}

template<usize N>
template<typename T>
T* Marena<N>::alloc(usize size, usize align) {
    uptr here = (uptr)mem + used;
    uptr offset = here % align;
    uptr next = here + (offset ? align - offset : 0);
    assert(next + size - (uptr)mem < N);
    T* ret = (T*)next;
    used += offset + size;
    high_water = _MAX(high_water, used);
    return ret;
}

template<typename U, usize N>
template<typename T>
T* Varena<U,N>::alloc(usize size, usize align) {
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

inline void* base_alloc(usize sz) {
    void* ret = calloc(sz, 1);
    assert(ret);
    allocs++;
    return ret;
}

inline void base_free(void* mem) {
    if(!mem) return;
    allocs--;
    free(mem);
}

inline void* virt_alloc(usize sz) {
    vallocs++;
#ifdef _WIN32
    void* ret = VirtualAlloc(null, sz, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    assert(ret);
#elif defined(__linux__) 
    void* ret = mmap(null, sz, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    assert(ret != MAP_FAILED);
#endif
    return ret;
}

inline void virt_free(void* mem) {
    if(!mem) return;
    vallocs--;
#ifdef _WIN32
    VirtualFree(mem, 0, MEM_RELEASE);
#elif defined(__linux__) 
    munmap(mem, 0);
#endif
}

inline void mem_validate() {
    if(allocs != 0 || vallocs != 0) {
        warn("Unbalanced allocations: %, (vert) %", allocs.load(), vallocs.load());
    } else {
        info("No memory leaked.");
    }
}
inline u32 __validator = atexit(mem_validate);
