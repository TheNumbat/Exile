
#pragma once

// TODO(max): should this be an actual stack?
#define PUSH_PROFILE(enable) bool __prof = this_thread_data.profiling; this_thread_data.profiling = enable;
#define POP_PROFILE() {this_thread_data.profiling = __prof;}

#ifdef _MSC_VER
#define POST_MSG(m) {PUSH_PROFILE(false) {(m).time = __rdtsc(); this_thread_data.dbg_msgs.push(m);} POP_PROFILE();}
#else
#error "Fix this"
#endif

#define PROF_SEC(n) 	_prof_sec(n, CONTEXT);
#define PROF_SEC_END() 	_prof_sec_end();

#define BEGIN_FRAME() { \
	dbg_msg msg; \
	msg.type = dbg_msg_type::begin_frame; \
	msg.begin_frame.perf = global_api->platform_get_perfcount(); \
	POST_MSG(msg); \
}
#define END_FRAME() { \
	dbg_msg msg; \
	msg.type = dbg_msg_type::end_frame; \
	msg.end_frame.perf = global_api->platform_get_perfcount(); \
	POST_MSG(msg); \
	global_dbg->collate(); \
}

typedef u64 clock;

enum class dbg_msg_type : u8 {
	none,
	begin_frame,
	end_frame,
	allocate,
	reallocate,
	free,
	enter_func,
	exit_func,
	mut_lock,
	mut_unlock,
	sem_wait,
	section_begin,
	section_end,
};

struct dbg_msg_begin_frame {
	platform_perfcount perf = 0;
};

struct dbg_msg_end_frame {
	platform_perfcount perf = 0;
};

struct dbg_msg_allocate {
	void* to 		 = null;
	u64 bytes 		 = 0;
	allocator* alloc = null;
};

struct dbg_msg_reallocate {
	void* to		 = null;
	void* from 		 = null;
	u64 bytes		 = 0;
	allocator* alloc = null;
};

struct dbg_msg_free {
	void* from 		 = null;
	allocator* alloc = null;
};

struct dbg_msg_enter_func {};

struct dbg_msg_exit_func {};

struct dbg_msg_mut_lock {
	platform_mutex* mut = null;
};

struct dbg_msg_mut_unlock {
	platform_mutex* mut = null;
};

struct dbg_msg_sem_wait {
	platform_semaphore* sem = null;
};

struct dbg_msg {
	dbg_msg_type type = dbg_msg_type::none;
	clock time = 0;
	code_context context;
	code_context call_stack[32];
	union {
		dbg_msg_begin_frame   begin_frame;	// done
		dbg_msg_end_frame 	  end_frame;	// done
		dbg_msg_allocate      allocate;		// done
		dbg_msg_reallocate    reallocate;	// done
		dbg_msg_free          free;			// done
		dbg_msg_enter_func    enter_func;	// done
		dbg_msg_exit_func     exit_func;	// done
		dbg_msg_mut_lock      mut_lock;
		dbg_msg_mut_unlock    mut_unlock;
		dbg_msg_sem_wait      sem_wait;
	};
	dbg_msg() {};
};

struct profile_node {
	code_context context;
	clock self = 0, heir = 0, begin = 0;
	u32 calls = 0;
	bool enabled = false;

	vector<profile_node*> children;
	CIRCULAR profile_node* parent = null; 
};

struct alloc_frame_profile {
	bool show = false;
	vector<dbg_msg> allocs;

	static alloc_frame_profile make(allocator* alloc);
	void destroy();
};

struct frame_profile {

	clock clock_start = 0, clock_end = 0;
	platform_perfcount perf_start = 0, perf_end = 0;
	vector<profile_node*> heads;
	profile_node* current = null;

	map<allocator*, alloc_frame_profile> allocations;

	pool_allocator pool;
	u32 number = 0;

	bool show_prof = true, show_allocs = true;

	void setup(string name, allocator* alloc, clock time, platform_perfcount perf, u32 num);
	void destroy();
};

struct single_alloc {
	code_context origin;
	u64 size = 0;
};

struct alloc_profile {

	map<void*, single_alloc> current_set; 

	u64 current_size = 0, total_allocated = 0, total_freed = 0;
	u64 num_allocs = 0, num_frees = 0, num_reallocs = 0;

	bool shown = false;

	static alloc_profile make(allocator* alloc);
	void destroy();
};

struct thread_profile {
	string name;
	queue<frame_profile> frames;
	
	u32 frame_buf_size = 0, num_frames = 0, frame_size = 0;
	i32 selected_frame = 1;

	static thread_profile make();
	void destroy();
};

enum class prof_sort_type : u8 {
	none,
	name,
	heir,
	self,
	calls,
};

struct dbg_manager {

	bool frame_pause = true, show_alloc_stats = false;
	platform_thread_id selected_thread;
	f32 last_frame_time = 0.0f;

	prof_sort_type prof_sort = prof_sort_type::heir;

	allocator* alloc = null;

	platform_mutex stats_mut;
	map<platform_thread_id, thread_profile> thread_stats;
	map<allocator*, alloc_profile> alloc_stats;
	alloc_profile alloc_totals;

	locking_queue<log_message> log_cache;
	log_level lvl = log_level::info;

	arena_allocator scratch;

///////////////////////////////////////////////////////////////////////////////

	static dbg_manager make(allocator* alloc);
	void destroy();

	void UI();
	void profile_recurse(vector<profile_node*> list);
	void fixdown_self_timings(profile_node* node);
	void process_frame_alloc_msg(frame_profile* frame, dbg_msg* msg);
	void process_alloc_msg(dbg_msg* msg);

	void shutdown_log(log_manager* log);
	void setup_log(log_manager* log);
	void register_thread(u32 frames, u32 frame_size);

	void collate();
};

void _prof_sec(string name, code_context context);
void _prof_sec_end();

CALLBACK void dbg_add_log(log_message* msg, void*);

bool prof_sort_name(profile_node* l, profile_node* r);
bool prof_sort_heir(profile_node* l, profile_node* r);
bool prof_sort_self(profile_node* l, profile_node* r);
bool prof_sort_calls(profile_node* l, profile_node* r);
bool operator<=(single_alloc l, single_alloc r);
