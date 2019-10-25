
#pragma once

#include <thread>

#include "lib/lib.h"

struct Profiler {

    void init();
    void destroy();

    static void begin_frame();
    static void end_frame();

    static void msg_alloc();
    static void msg_dealloc();
    static void msg_enter();
    static void msg_exit();

    // TODO(max): thread sychrononization profiling?

private:

    struct allocation {

    };

    struct alloc_profile {

    };

    struct timing_node {
        vec_view<timing_node> children;
    };

    struct frame_profile {

        void init();
        void destroy();
        
        Varena<> arnea;
        timing_node root;
        vec_view<allocation> allocations;
    };

    struct thread_profile {
        queue<frame_profile> frames;
    };

    map<std::thread::id, thread_profile> threads;
    map<literal, alloc_profile> allocators;
};
