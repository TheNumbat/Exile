
#include "lib.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif defined(__linux__)
    #include <sys/mman.h>
#endif

#include <atomic>

std::atomic<i64> allocs = 0;
std::atomic<i64> vallocs = 0;

void* base_alloc(usize sz) {
    void* ret = calloc(sz, 1);
    assert(ret);
    allocs++;
    return ret;
}

void base_free(void* mem) {
    if(!mem) return;
    allocs--;
    free(mem);
}

void* virt_alloc(usize sz) {
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

void virt_free(void* mem) {
    if(!mem) return;
    vallocs--;
#ifdef _WIN32
    VirtualFree(mem, 0, MEM_RELEASE);
#elif defined(__linux__) 
    munmap(mem, 0);
#endif
}

void mem_validate() {
    if(allocs != 0 || vallocs != 0) {
        warn("Unbalanced allocations: %, (vert) %", allocs.load(), vallocs.load());
    } else {
        info("No memory leaked.");
    }
}
