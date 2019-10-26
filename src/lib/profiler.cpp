
#include "lib.h"

#ifdef __clang__
#include <x86intrin.h>
#endif

bool operator==(const Profiler::Location& l, const Profiler::Location& r) {
    return l.line == r.line && l.file == r.file && l.func == r.func;
}

u64 Profiler::timestamp() {
    return __rdtsc();
}

void Profiler::destroy() {
    std::lock_guard lock(threads_lock);
    threads.destroy();
    allocs.destroy();
}

void Profiler::start_thread() {
    std::lock_guard lock(threads_lock);

    thread_id id = std::this_thread::get_id();
    assert(!threads.try_get(id));

    threads.insert(id, &this_thread);
}

void Profiler::end_thread() {
    std::lock_guard lock(threads_lock);

    thread_id id = std::this_thread::get_id();
    assert(threads.try_get(id));

    threads.erase(id);
    this_thread.destroy();
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
    allocations = vec_view<Alloc>::make(arena, max_allocs);
    root->children = vec_view<timing_node>::make(arena, max_children);
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
    self_time = heir_time - child_time;
}

void Profiler::begin_frame() {

    thread_profile& prof = this_thread;

    if(!prof.frames.empty() && prof.frames.full()) {
        frame_profile f = prof.frames.pop();
        f.destroy();
    }

    frame_profile* new_frame = prof.frames.push({});
    new_frame->begin();

    during_frame = true;
}

void Profiler::end_frame() {

    thread_profile& prof = this_thread;

    assert(!prof.frames.empty());
    
    frame_profile* this_frame = prof.frames.back();
    this_frame->end();

    during_frame = false;
}

void Profiler::enter(literal l) {
#if PROFILE == 1 
    enter({l, "", 0});
#endif
}

void Profiler::enter(Location l) {
#if PROFILE == 1 
    this_thread.frames.back()->enter(l);
#endif
}

void Profiler::frame_profile::enter(Location l) {

    bool repeat = false;
    for(auto& n : current->children) {
        if(n.loc == l) {
            current = &n;
            repeat = true;
        }
    }

    if(!repeat) {
        assert(!current->children.full());
        timing_node& new_child = current->children.push({});
        new_child.parent = current;
        new_child.children = vec_view<timing_node>::make(arena, max_children);
        current = &new_child;
    }

    current->begin = timestamp();
    current->calls++;
}

void Profiler::exit() {
    this_thread.frames.back()->exit();
}

void Profiler::frame_profile::exit() {

    frame_profile* frame = this_thread.frames.back();
    timing_node* current = frame->current;

    current->end = timestamp();
    current->heir_time += current->end - current->begin;
    frame->current = current->parent;
}

void Profiler::alloc(Alloc a) {

#if PROFILE == 0
    return;
#endif

    if(during_frame) this_thread.frames.back()->allocations.push(a);

    std::lock_guard lock(allocs_lock);

    if(a.size) {
        
        alloc_profile* prof = allocs.try_get(a.name);
        if(!prof) {
            prof = &allocs.insert(a.name, {});
        }

        if(prof->current_set.try_get(a.addr)) {
            i64* monkas = prof->current_set.try_get(a.addr);
        }
        prof->current_set.insert(a.addr, a.size);

        prof->allocate_size += a.size;
        prof->allocates++;
        prof->current_set_size += a.size;

    } else {
        
        alloc_profile& prof = allocs.get(a.name);
        i64 size = prof.current_set.get(a.addr);
        prof.current_set.erase(a.addr);

        prof.free_size += size;
        prof.frees++;
        prof.current_set_size -= size;
    }
}
