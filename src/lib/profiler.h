
#pragma once

// NOTE(max): if PROFILE is defined to 0, enter, exit, and alloc will no-op,
// but begin_frame and end_frame will still keep track of frame timing, and
// data structures will be allocated.
#define PROFILE 1

#define Here (Location{__func__, last_file(literal(__FILE__)), (usize)__LINE__})

struct Location {
    literal func, file;
    usize line = 0;
};

struct Profiler {

    static void start_thread();
    static void end_thread();

    static void begin_frame();
    static void end_frame();

    using thread_id = std::thread::id;

    struct Alloc {
        literal name;
        void* addr = null;
        usize size = 0; // 0 implies free
    };

    struct Timing_Node {
        Location loc;
        
        void compute_times();
        u64 begin = 0, end = 0;
        u64 self_time = 0, heir_time = 0;
        u64 calls = 0;
        
        void visit(std::function<void(Timing_Node)> f);
        vec_view<Timing_Node> children;
        Timing_Node* parent = null;
    };

    static void enter(Location l);
    static void enter(literal l);
    static void exit();

    static void alloc(Alloc a);

    static u64 timestamp();
    static f64 ms(u64 cnt);

    static void iterate_timings(std::function<void(thread_id, Timing_Node)> f);
    
    // called atexit
    static void destroy();
private:

    static constexpr char prof_vname[] = "VProfiler";
    using prof_valloc = Mvallocator<prof_vname, false>;

    struct Alloc_Profile {
        void destroy();
        i64 allocates = 0, frees = 0;
        i64 allocate_size = 0, free_size = 0;
        i64 current_set_size = 0;
        map<void*, i64, Mhidden> current_set;
    };

    struct Frame_Profile {
        static constexpr usize max_allocs = 4096;
        static constexpr usize max_children = 16;

        void begin();
        void end();
        void enter(Location l);
        void exit();
        void destroy();

        Varena<prof_valloc> arena;
        Timing_Node* root = null;
        Timing_Node* current = null;
        vec_view<Alloc> allocations;
    };

    struct Thread_Profile {
        void destroy();

        bool during_frame = false;
        queue<Frame_Profile, Mhidden> frames;
    };

    static inline std::mutex threads_lock;
    static inline std::mutex allocs_lock;
    static inline thread_local Thread_Profile this_thread;

    static inline map<thread_id, Thread_Profile*, Mhidden> threads;
    static inline map<literal, Alloc_Profile, Mhidden> allocs;
};

static inline u32 __prof_destroy = atexit(Profiler::destroy);

u32 hash(const Location& l);
bool operator==(const Location& l, const Location& r);

template<>
struct Type_Info<Location> {
	static constexpr char name[] = "Location";
	static constexpr usize size = sizeof(Location);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _func[] = "func";
    static constexpr char _file[] = "file";
    static constexpr char _line[] = "line";
	using members = Type_List<Record_Field<literal,offset_of(func, Location),_func>,
                              Record_Field<literal,offset_of(file, Location),_file>,
                              Record_Field<usize,offset_of(line, Location),_line>>;
};

template<>
struct Type_Info<Profiler::Alloc> {
	static constexpr char name[] = "Alloc";
	static constexpr usize size = sizeof(Profiler::Alloc);
	static constexpr Type_Type type = Type_Type::record_;
    static constexpr char _name[] = "name";
    static constexpr char _addr[] = "addr";
    static constexpr char _size[] = "size";
	using members = Type_List<Record_Field<literal,offset_of(name, Profiler::Alloc),_name>,
                              Record_Field<void*,offset_of(addr, Profiler::Alloc),_addr>,
                              Record_Field<usize,offset_of(size, Profiler::Alloc),_size>>;
};
