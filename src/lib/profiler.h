
#pragma once

// NOTE(max): if PROFILE is defined to 0, enter, exit, and alloc will no-op,
// but begin_frame and end_frame will still keep track of frame timing, and
// data structures will be allocated.
#define PROFILE 1

#define Here (Profiler::Location{__func__, last_file(literal(__FILE__)), (usize)__LINE__})

struct Profiler {

    static void start_thread();
    static void end_thread();

    static void begin_frame();
    static void end_frame();

    struct Location {
        literal func, file;
        usize line = 0;
    };

    struct Alloc {
        literal name;
        void* addr = null;
        usize size = 0; // 0 implies free
    };

    static void enter(Location l);
    static void enter(literal l);
    static void exit();

    static void alloc(Alloc a);

    static u64 timestamp();
    
    // called atexit
    static void destroy();
private:

    static constexpr char prof_vname[] = "VProfiler";
    using prof_valloc = Mvallocator<prof_vname>;

    using thread_id = std::thread::id;

    struct alloc_profile {
        void destroy();
        i64 allocates = 0, frees = 0;
        i64 allocate_size = 0, free_size = 0;
        i64 current_set_size = 0;
        map<void*, i64, Mhidden> current_set;
    };

    struct timing_node {
        Location loc;
        
        void compute_times();
        u64 begin = 0, end = 0;
        u64 self_time = 0, heir_time = 0;
        u64 calls = 0;
        
        vec_view<timing_node> children;
        timing_node* parent = null;
    };

    struct frame_profile {
        static constexpr usize max_allocs = 4096;
        static constexpr usize max_children = 16;

        void begin();
        void end();
        void enter(Location l);
        void exit();
        void destroy();

        Varena<prof_valloc> arena;
        timing_node* root = null;
        timing_node* current = null;
        vec_view<Alloc> allocations;
    };

    struct thread_profile {
        void destroy();
        queue<frame_profile, Mhidden> frames;
    };

    static inline std::mutex threads_lock;
    static inline std::mutex allocs_lock;
    static inline thread_local thread_profile this_thread;

    static inline bool during_frame = false;
    static inline map<thread_id, thread_profile*, Mhidden> threads;
    static inline map<literal, alloc_profile, Mhidden> allocs;
};

static inline u32 __prof_destroy = atexit(Profiler::destroy);

bool operator==(const Profiler::Location& l, const Profiler::Location& r);
