
#include "lib.h"
#include <atomic>

std::atomic<i64> allocs = 0;
std::atomic<i64> vallocs = 0;

u8* base_alloc(usize sz) {
    u8* ret = (u8*)calloc(sz, 1);
    assert(ret);
    allocs++;
    return ret;
}

void base_free(void* mem) {
    if(!mem) return;
    allocs--;
    free(mem);
}

void mem_validate() {
    if(allocs != 0 || vallocs != 0) {
        warn("Unbalanced allocations: %, (vert) %", allocs.load(), vallocs.load());
    } else {
        info("No memory leaked.");
    }
}
