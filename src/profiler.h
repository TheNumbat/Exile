
#pragma once

#include <thread>

#include "lib/lib.h"

#define Context (Profiler::Location{__func__, __FILE__, (usize)__LINE__})

struct Profiler {

    static void start_thread();
    static void end_thread();

    static void destroy();

    static void begin_frame();
    static void end_frame();

    static void alloc();
    static void dealloc();

    struct Location {
        literal func, file;
        usize line = 0;
    };

    static void enter(Location l);
    static void enter(literal l);
    static void exit();

    static u64 timestamp();

private:

    static constexpr char prof_name[] = "Profiler";
    static constexpr char prof_vname[] = "VProfiler";
    using prof_alloc = Mallocator<prof_name>;
    using prof_valloc = Mvallocator<prof_vname>;

    using thread_id = std::thread::id;

    struct alloc_profile {
        void destroy();
        i64 allocates = 0, frees = 0;
        i64 allocate_size = 0, free_size = 0;
        i64 current_set_size = 0;
        map<void*, usize> current_set;
    };

    struct timing_node {
        void compute_times();
        u64 begin = 0, end = 0;
        u64 self_time = 0, heir_time = 0;
        u64 calls = 0;
        vec_view<timing_node> children;
        timing_node* parent = null;
    };

    struct allocation {
        void* address = 0;
        usize size = 0;
    };

    struct frame_profile {
        static constexpr usize max_allocs = 4096;
        static constexpr usize max_children = 16;

        void begin();
        void end();
        void destroy();   

        Varena<prof_valloc> arena;
        timing_node* root = null;
        timing_node* current = null;
        vec_view<allocation> allocations;
    };

    struct thread_profile {
        void destroy();
        queue<frame_profile,prof_alloc> frames;
    };

    static inline std::mutex threads_view_lock;
    static inline thread_local thread_profile this_thread_profile;

    static inline map<thread_id, thread_profile*, prof_alloc> threads_view;
    static inline map<literal, alloc_profile, prof_alloc> allocators;
};
