
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
    root = arena.make<timing_node>();
    current = root;
    root->begin = timestamp();
}

void Profiler::frame_profile::end() {
    assert(current == root);
    root->end = timestamp();
    root->compute_times();
}

void Profiler::timing_node::compute_times() {
    u64 child_time = 0;
    for(auto& c : children) {
        c.compute_times();
        child_time += c.heir_time;
    }
    heir_time = end - begin;
    self_time = heir_time - child_time;
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

    assert(!prof.frames.empty());
    
    frame_profile* this_frame = prof.frames.back();
    this_frame->end();
}

void Profiler::alloc() {

}

void Profiler::dealloc() {

}

void Profiler::enter() {

}

void Profiler::exit() {

}
