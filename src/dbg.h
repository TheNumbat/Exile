
#pragma once

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

};

struct dbg_msg_end_frame {

};

struct dbg_msg_collate_frame {

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

struct dbg_msg_enter_func {
	code_context func;
};

struct dbg_msg_exit_func {

};

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
	code_context context;
	union {
		dbg_msg_begin_frame   begin_frame;
		dbg_msg_end_frame 	  end_frame;
		dbg_msg_collate_frame collate_frame;
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

struct dbg_manager {

	u32 current_frame = 0;
	bool really_running = false;

	queue<log_message> log_cache;
	log_level lvl = log_level::info;

	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static dbg_manager make(log_manager* log, allocator* alloc);
	void destroy();

	void collate();
	void render_debug_gui(platform_window* win);
};

CALLBACK void dbg_add_log(log_message* msg);
