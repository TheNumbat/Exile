
#include "lib.h"
#include <SDL2/SDL.h>

u32 hash(Location l) {
    return hash(l.func) ^ hash(l.file) ^ hash(l.line);
}

bool operator==(const Location& l, const Location& r) {
    return l.line == r.line && l.file == r.file && l.func == r.func;
}

u64 Profiler::timestamp() {
    return (u64)SDL_GetPerformanceCounter();
}

f64 Profiler::ms(u64 cnt) {
    return cnt / (f64)SDL_GetPerformanceFrequency();
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

void Profiler::Alloc_Profile::destroy() {
    current_set.destroy();
    allocates = 0, frees = 0;
    allocate_size = 0, free_size = 0;
    current_set_size = 0;
} 

void Profiler::Thread_Profile::destroy() {
    frames.destroy();
}

void Profiler::Frame_Profile::destroy() {
    arena.reset();
    current = null;
    root = {};
    allocations.destroy();
}

void Profiler::Frame_Profile::begin() {
    root = arena.make<Timing_Node>();
    current = root;
    root->begin = timestamp();
    root->loc = {"Main Loop", "", 0};
    allocations = vec_view<Alloc>::make(arena, max_allocs);
    root->children = vec_view<Timing_Node>::make(arena, max_children);
}

void Profiler::Frame_Profile::end() {
    assert(current == root);
    root->end = timestamp();
    root->heir_time = root->end - root->begin;
    root->compute_times();
}

void Profiler::Timing_Node::compute_times() {
    u64 child_time = 0;
    for(auto& c : children) {
        c.compute_times();
        child_time += c.heir_time;
    }
    self_time = heir_time - child_time;
}

void Profiler::begin_frame() {

    Thread_Profile& prof = this_thread;

    if(!prof.frames.empty() && prof.frames.full()) {
        Frame_Profile f = prof.frames.pop();
        f.destroy();
    }

    Frame_Profile* new_frame = prof.frames.push({});
    new_frame->begin();

    prof.during_frame = true;
}

void Profiler::end_frame() {

    Thread_Profile& prof = this_thread;

    assert(!prof.frames.empty());
    
    Frame_Profile* this_frame = prof.frames.back();
    this_frame->end();

    prof.during_frame = false;
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

void Profiler::Frame_Profile::enter(Location l) {

    bool repeat = false;
    for(auto& n : current->children) {
        if(n.loc == l) {
            current = &n;
            repeat = true;
        }
    }

    if(!repeat) {
        assert(!current->children.full());
        Timing_Node& new_child = current->children.push({});
        new_child.parent = current;
        new_child.children = vec_view<Timing_Node>::make(arena, max_children);
        current = &new_child;
    }

    current->begin = timestamp();
    current->calls++;
}

void Profiler::exit() {
    this_thread.frames.back()->exit();
}

void Profiler::Frame_Profile::exit() {

    Frame_Profile* frame = this_thread.frames.back();
    Timing_Node* current = frame->current;

    current->end = timestamp();
    current->heir_time += current->end - current->begin;
    frame->current = current->parent;
}

void Profiler::alloc(Alloc a) {

#if PROFILE == 0
    return;
#endif

    if(this_thread.during_frame) this_thread.frames.back()->allocations.push(a);

    std::lock_guard lock(allocs_lock);
    Alloc_Profile& prof = allocs.get_or_insert(a.name);

    if(a.size) {

        if(prof.current_set.try_get(a.addr)) {
            i64* monkas = prof.current_set.try_get(a.addr);
        }
        prof.current_set.insert(a.addr, a.size);

        prof.allocate_size += a.size;
        prof.allocates++;
        prof.current_set_size += a.size;

    } else {
        
        i64 size = prof.current_set.get(a.addr);
        prof.current_set.erase(a.addr);

        prof.free_size += size;
        prof.frees++;
        prof.current_set_size -= size;
    }
}

void Profiler::Timing_Node::visit(std::function<void(Timing_Node)> f) {
    f(*this);
    for(auto& n : children) n.visit(f);
}

void Profiler::iterate_timings(std::function<void(thread_id, Timing_Node)> f) {

    std::lock_guard lock(threads_lock);

    for(auto& entry : threads) {

        thread_id id = entry.key;
        Thread_Profile* tp = entry.value;

        Frame_Profile* fp = null;
        if(tp->during_frame && tp->frames.size > 1) {
            fp = tp->frames.penultimate();
        } else if(!tp->during_frame && tp->frames.empty()) {
            fp = tp->frames.back();
        } else continue;

        fp->root->visit([&f, id](Timing_Node n) {return f(id, n);});
    }    
}
