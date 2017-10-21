
#pragma once

// TODO(max): should this be an actual stack?
#define PUSH_PROFILE(enable) bool __prof = this_thread_data.profiling; this_thread_data.profiling = enable;
#define POP_PROFILE() {this_thread_data.profiling = __prof;}

#define POST_MSG(m) {PUSH_PROFILE(false) {this_thread_data.dbg_msgs.push(m);} POP_PROFILE();}

typedef u64 timestamp;

struct func_profile_node {
	code_context context;
	timestamp self = 0, heir = 0;
	u32 calls = 0;

	vector<func_profile_node*> children;
	CIRCULAR func_profile_node* parent = null; 
};

struct frame_profile {

	timestamp start = 0, end = 0;
	vector<func_profile_node*> heads;
	func_profile_node* current = null;
	// vector<func_profile_node*> self_time_view, heir_time_view, calls_view;
	// vector<dbg_msg*> allocations;
	arena_allocator arena;
};

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

struct dbg_msg_section_begin {
	string sec;
};

struct dbg_msg_section_end {

};

struct dbg_msg {
	dbg_msg_type type = dbg_msg_type::none;
	timestamp t = 0;
	code_context context;
	union {
		dbg_msg_begin_frame   begin_frame;
		dbg_msg_end_frame 	  end_frame;
		dbg_msg_allocate      allocate;
		dbg_msg_reallocate    reallocate;
		dbg_msg_free          free;
		dbg_msg_enter_func    enter_func;
		dbg_msg_exit_func     exit_func;
		dbg_msg_mut_lock      mut_lock;
		dbg_msg_mut_unlock    mut_unlock;
		dbg_msg_sem_wait      sem_wait;
		dbg_msg_section_begin section_begin;
		dbg_msg_section_end   section_end;
	};
	dbg_msg() {};
};

struct thread_profile {
	queue<frame_profile> frames;
	u32 frame_buf_size = 0, num_frames = 0, frame_size = 0;
};

struct dbg_manager {

	u32 current_frame = 0;
	allocator* alloc = null;

	platform_mutex cache_mut;
	map<platform_thread_id, thread_profile> dbg_cache;

///////////////////////////////////////////////////////////////////////////////

	static dbg_manager make(allocator* alloc);
	void destroy();

	void register_thread(u32 frames, u32 frame_size);

	void collate();
};

