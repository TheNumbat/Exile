
#pragma once

enum class dbg_msg_type : u8 {
	none,
	frame_begin,
	frame_end,
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

struct dbg_msg_frame_begin {

};

struct dbg_msg_frame_end {

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
	string func;
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
		dbg_msg_frame_begin   frame_begin;
		dbg_msg_frame_end     frame_end;
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
};

struct dbg_frame {
	vector<dbg_msg> messages;
};

struct dbg_manager {

	vector<dbg_frame> frames;

	vector<log_message> log_cache;
	log_level lvl = log_level::info;

	allocator* alloc = null;
};

dbg_manager make_dbg_manager(log_manager* log, allocator* alloc);
void destroy_dbg_manager(dbg_manager* dbg);

void dbg_add_log(log_message* msg);
void render_debug_gui(platform_window* win, dbg_manager* dgb);
