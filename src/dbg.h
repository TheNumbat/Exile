
#pragma once

// TODO(max): should this be an actual stack?
#define PUSH_PROFILE(enable) bool __prof = this_thread_data.profiling; this_thread_data.profiling = enable;
#define POP_PROFILE() {this_thread_data.profiling = __prof;}

#ifdef _MSC_VER
#define POST_MSG(m) {PUSH_PROFILE(false) {(m).time = __rdtsc(); this_thread_data.dbg_msgs.push(m);} POP_PROFILE();}
#else
#define POST_MSG(m) {PUSH_PROFILE(false) {(m).time = global_api->platform_get_perfcount(); this_thread_data.dbg_msgs.push(m);} POP_PROFILE();}
#endif

#define PROF_SEC(n) _prof_sec(n, CONTEXT);
#define PROF_SEC_END() _prof_sec_end();

#define BEGIN_FRAME() { \
	dbg_msg msg; \
	msg.type = dbg_msg_type::begin_frame; \
	POST_MSG(msg); \
}
#define END_FRAME() { \
	dbg_msg msg; \
	msg.type = dbg_msg_type::end_frame; \
	POST_MSG(msg); \
}

typedef platform_perfcount timestamp;

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

struct dbg_msg_begin_frame {};

struct dbg_msg_end_frame {};

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

struct dbg_msg_section_begin {};
struct dbg_msg_section_end {};

struct dbg_msg {
	dbg_msg_type type = dbg_msg_type::none;
	timestamp time = 0;
	code_context context;
	union {
		dbg_msg_begin_frame   begin_frame;	// done
		dbg_msg_end_frame 	  end_frame;	// done
		dbg_msg_allocate      allocate;
		dbg_msg_reallocate    reallocate;
		dbg_msg_free          free;
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
	timestamp self = 0, heir = 0, begin = 0;
	u32 calls = 0;
	bool enabled = false;

	vector<profile_node*> children;
	CIRCULAR profile_node* parent = null; 
};

struct frame_profile {

	timestamp start = 0, end = 0;
	vector<profile_node*> heads;
	profile_node* current = null;

	map<allocator*, vector<dbg_msg>> allocations;

	pool_allocator pool;
	u32 number = 0;
};

struct alloc_profile {
	u64 size = 0, allocated = 0, freed = 0;
	u64 num_allocs = 0, num_frees = 0, num_reallocs = 0;
};

struct thread_profile {
	string name;
	queue<frame_profile> frames;
	u32 frame_buf_size = 0, num_frames = 0, frame_size = 0;
};

enum class prof_sort_type : u8 {
	none,
	name,
	heir,
	self,
	calls,
};

struct dbg_manager {

	bool frame_pause = true;
	i32 selected_frame = 1;
	platform_thread_id selected_thread;

	prof_sort_type prof_sort = prof_sort_type::name;

	allocator* alloc = null;

	platform_mutex cache_mut;
	map<platform_thread_id, thread_profile> dbg_cache;

	platform_mutex alloc_mut;
	map<allocator*, alloc_profile> alloc_stats;

	queue<log_message> log_cache;
	log_level lvl = log_level::info;

	arena_allocator scratch;

///////////////////////////////////////////////////////////////////////////////

	static dbg_manager make(allocator* alloc);
	void destroy();

	void UI();
	void profile_recurse(vector<profile_node*> list);
	void fixdown_self_timings(profile_node* node);

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
