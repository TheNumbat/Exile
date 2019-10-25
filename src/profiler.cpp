
#include "profiler.h"

#ifdef __clang__
#include <x86intrin.h>
#endif

u64 Profiler::timestamp() {
    return __rdtsc();
}

void Profiler::destroy() {
    std::lock_guard lock(threads_view_lock);
    threads_view.destroy();
    allocators.destroy();
}

void Profiler::start_thread() {
    std::lock_guard lock(threads_view_lock);

    thread_id id = std::this_thread::get_id();
    assert(!threads_view.try_get(id));

    threads_view.insert(id, &this_thread_profile);
}

void Profiler::end_thread() {
    std::lock_guard lock(threads_view_lock);

    thread_id id = std::this_thread::get_id();
    assert(threads_view.try_get(id));

    threads_view.erase(id);

    this_thread_profile.destroy();
}

void Profiler::alloc_profile::destroy() {
    current_set.destroy();
    allocates = 0, frees = 0;
    allocate_size = 0, free_size = 0;
    current_set_size = 0;
} 

void Profiler::thread_profile::destroy() {
    frames.destroy();
}

void Profiler::frame_profile::destroy() {
    arena.reset();
    current = null;
    root = {};
    allocations.destroy();
}

void Profiler::frame_profile::begin() {
    current = &root;
    root.begin = timestamp();
}

void Profiler::begin_frame() {

    thread_profile& prof = this_thread_profile;

    if(!prof.frames.empty() && prof.frames.full()) {
        frame_profile f = prof.frames.pop();
        f.destroy();
    }

    frame_profile* new_frame = prof.frames.push({});
    new_frame->begin();
}

void Profiler::end_frame() {

    thread_profile& prof = this_thread_profile;
}

void Profiler::alloc() {

}

void Profiler::dealloc() {

}

void Profiler::enter() {

}

void Profiler::exit() {

}
