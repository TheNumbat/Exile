
#pragma once

#include <thread>

#include "lib/lib.h"

struct Profiler {

    static void destroy();
    static void init_this_thread();

    static void begin_frame();
    static void end_frame();

    static void msg_alloc();
    static void msg_dealloc();
    static void msg_enter();
    static void msg_exit();

    // TODO(max): thread sychrononization profiling?

private:

    using thread_id = std::thread::id;

    struct allocation {

    };

    struct alloc_profile {
        void destroy();
    };

    struct timing_node {
        vec_view<timing_node> children;
    };

    struct frame_profile {
        void destroy();        
        Varena<> arnea;
        timing_node root;
        vec_view<allocation> allocations;
    };

    struct thread_profile {
        void destroy();
        queue<frame_profile> frames;
    };

    static map<thread_id, thread_profile> threads;
    static map<literal, alloc_profile> allocators;
};
