
#include "profiler.h"

void Profiler::destroy() {
    threads.destroy();
    allocators.destroy();
}

void Profiler::init_this_thread() {

    thread_id id = std::this_thread::get_id();

    assert(!threads.try_get(id));
    threads.insert(id, {});
}

void Profiler::alloc_profile::destroy() {

}

void Profiler::frame_profile::destroy() {

}

void Profiler::thread_profile::destroy() {

}
