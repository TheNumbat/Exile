
#include "alloc.h"
#include "log.h"

#include <atomic>
#include <string.h>

std::atomic<i64> allocs = 0;

u8* galloc(usize sz) {
    u8* ret = (u8*)calloc(sz, 1);
    assert(ret);
    allocs++;
    return ret;
}

void gfree(void* mem) {
    free(mem);
    allocs--;
}

void mem_validate() {
    if(allocs != 0) {
        warn("Unbalanced allocations: %lld", allocs.load());
    } else {
        info("No memory leaked.");
    }
}