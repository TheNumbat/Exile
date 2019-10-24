
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
        vec<timing_node*,Mframe> children;
    };

    struct frame_profile {
        static constexpr char a_name[] = "frame_profiles";
        using Mframe = MVarena<a_name>;

        void init();
        void destroy();
        timing_node root;
        vec<allocation,Mframe> allocations;
    };

    struct thread_profile {
        queue<frame_profile> frames;
    };

    map<std::thread::id, thread_profile> threads;
    map<literal, alloc_profile> allocators;
};
