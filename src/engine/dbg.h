
#pragma once

// TODO(max): should this be an actual stack?
#define PUSH_PROFILE(enable) bool __prof = this_thread_data.profiling; this_thread_data.profiling = enable;
#define POP_PROFILE() {this_thread_data.profiling = __prof;}
// TODO(max): this is a bit evil
#define PUSH_PROFILE_PROF(enable) bool __pprof = this_thread_data.timing_override; this_thread_data.timing_override = enable;
#define POP_PROFILE_PROF() {this_thread_data.timing_override = __pprof;}

#ifdef _MSC_VER
#define POST_MSG(m) {PUSH_PROFILE(false) {(m).time = __rdtsc(); this_thread_data.dbg_queue.push(m);} POP_PROFILE();}
#else
#error "Fix this"
#endif

#ifdef PROFILE
#define PROF_SEC(n) 	_prof_sec(n, CONTEXT);
#define PROF_SEC_END() 	_prof_sec_end();
#else
#define PROF_SEC(n)
#define PROF_SEC_END() 
#endif

#define BEGIN_FRAME() { \
	dbg_msg msg; \
	msg.type = dbg_msg_type::begin_frame; \
	msg.begin_frame.perf = global_api->get_perfcount(); \
	POST_MSG(msg); \
}
#define END_FRAME() { \
	dbg_msg msg; \
	msg.type = dbg_msg_type::end_frame; \
	msg.end_frame.perf = global_api->get_perfcount(); \
	POST_MSG(msg); \
	global_dbg->profiler.collate(); \
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
	u64 perf = 0;
};

struct dbg_msg_end_frame {
	u64 perf = 0;
};

struct dbg_msg_allocate {
	void* to 		 = null;
	u64 bytes 		 = 0;
	allocator alloc;
};

struct dbg_msg_reallocate {
	void* to		 = null;
	void* from 		 = null;
	u64 to_bytes	 = 0;
	u64 from_bytes 	 = 0;
	allocator alloc;
};

struct dbg_msg_free {
	void* from 		 = null;
	u64 bytes 		 = 0;
	allocator alloc;
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
	union {
		dbg_msg_begin_frame   begin_frame;
		dbg_msg_end_frame 	  end_frame;
		dbg_msg_allocate      allocate;
		dbg_msg_reallocate    reallocate;
		dbg_msg_free          free;
		dbg_msg_enter_func    enter_func;
		dbg_msg_exit_func     exit_func;
		dbg_msg_mut_lock      mut_lock;		// TODO(max)
		dbg_msg_mut_unlock    mut_unlock;	// TODO(max)
		dbg_msg_sem_wait      sem_wait;		// TODO(max)
	};
	dbg_msg() {};
};
bool operator>(dbg_msg& l, dbg_msg& r);

struct profile_node {
	code_context context;
	clock self = 0, heir = 0, begin = 0;
	u32 calls = 0;

	vector<profile_node*> children;
	CIRCULAR profile_node* parent = null;
};

struct alloc_frame_profile {
	vector<dbg_msg> allocs;

	static alloc_frame_profile make(allocator* alloc);
	void destroy();
};

struct frame_profile {

	clock clock_start = 0, clock_end = 0;
	u64 perf_start = 0, perf_end = 0;
	vector<profile_node*> heads;
	profile_node* current = null;

	map<allocator, alloc_frame_profile> allocations;

	pool_allocator pool;
	u32 number = 0;

	void setup(string name, allocator* alloc, clock time, u64 perf, u32 num);
	void destroy();
};

struct addr_info {
	code_context last_loc;
	i64 size  = 0;
};

struct alloc_profile {

	map<void*, addr_info> current_set; 

	i64 current_size = 0;
	u64 total_allocated = 0, total_freed = 0;
	u64 num_allocs = 0, num_frees = 0, num_reallocs = 0;

	platform_mutex mut;

	static alloc_profile make(allocator* alloc);
	static alloc_profile* make_new(allocator* alloc);
	void destroy();
};

struct thread_profile {

	string name;
	queue<frame_profile> frames;
	platform_mutex mut;

	u32 frame_buf_size = 0, num_frames = 0;
	i32 selected_frame = 1;
	bool in_frame = false;

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

enum class dbg_value_class : u8 {
	section,
	edit,
	view,
	callback // NOTE(max): can also be used for "reactive" values
};

struct dbg_value;
struct dbg_value_sec {
	map<string, dbg_value> children;
};

struct dbg_value_cal {
	func_ptr<void, void*> callback;
	void* callback_param = null;
};

struct dbg_value {
	dbg_value_class type = dbg_value_class::section;
	union {
		any edit;
		any view;
		dbg_value_sec sec;
		dbg_value_cal cal;
	};
	dbg_value() {}
	static dbg_value make_edit(any a);
	static dbg_value make_view(any a);
	static dbg_value make_sec(allocator* alloc);
	static dbg_value make_cal(_FPTR* c, void* p);
	void destroy(allocator* alloc);
};

struct dbg_profiler {

	bool frame_pause = true;
	prof_sort_type prof_sort = prof_sort_type::heir;

	platform_thread_id selected_thread;
	f32 last_frame_time = 0.0f;

	platform_mutex stats_map_mut;
	map<platform_thread_id, thread_profile*> thread_stats;
	
	platform_mutex alloc_map_mut;
	map<allocator, alloc_profile*> alloc_stats;

	allocator* alloc = null;

	static dbg_profiler make(allocator* alloc);
	void destroy();

	void register_thread(u32 frames);

	void UI(platform_window* window);
	void recurse(vector<profile_node*> list);
	
	alloc_profile get_totals();

	void collate();
	void collate_allocs();
	void collate_threads();

	void fixdown_self_timings(profile_node* node);
	void process_frame_alloc_msg(frame_profile* frame, dbg_msg* msg);
	void process_alloc_msg(dbg_msg* msg);
	void print_remaining();
};

struct dbg_value_store {
	
	dbg_value value_store;
	allocator* alloc = null;

	static dbg_value_store make(allocator* alloc);
	void destroy();

	void add_ele(string path, _FPTR* callback, void* param = null);
	template<typename T> void add_var(string path, T* value);
	template<typename T> void add_val(string path, T* value);
	template<typename T> T get_var(string path);

	void UI(platform_window* window);
	void recurse(map<string, dbg_value> store);
};

struct console_msg {
	log_level lvl = log_level::console;
	string msg;
};

struct console_cmd {
	func_ptr<void, string, void*> func;
	void* param = null;
};

struct dbg_console {

	char input_buffer[1024] = {};

	ImGuiTextFilter filter;
	log_level base_level = log_level::debug;

	bool scroll_bottom = true;
	bool copy_clipboard = false;

	queue<console_msg> lines;
	platform_mutex lines_mut;

	log_level lvl = log_level::info; 

	vector<string> candidates;
	map<string, console_cmd> commands;

	allocator* alloc = null;

	void init(allocator* alloc);
	void destroy();

	void UI(platform_window* window);
	void on_text_edit(ImGuiTextEditCallbackData* data);
	
	void exec_command(string cmd);
	void add_console_msg(string msg);
	void add_command(string name, _FPTR* func, void* param = null);

	void shutdown_log(log_manager* log);
	void setup_log(log_manager* log);
};

struct dbg_manager {

	bool show_ui = false, show_profile = false, show_vars = false, show_console = true;

	dbg_console console;
	dbg_profiler profiler;
	dbg_value_store store;

///////////////////////////////////////////////////////////////////////////////

	void init(allocator* alloc);
	void destroy();
	void destroy_prof();

	void UI(platform_window* window);

	void toggle_profile();
	void toggle_vars();
	void toggle_console();
};

void _prof_sec(string name, code_context context);
void _prof_sec_end();

CALLBACK void dbg_reup_window(void* eng);
CALLBACK void dbg_add_log(log_message* msg, void*);
CALLBACK void console_cmd_clear(string, void* data);

bool prof_sort_name(profile_node* l, profile_node* r);
bool prof_sort_heir(profile_node* l, profile_node* r);
bool prof_sort_self(profile_node* l, profile_node* r);
bool prof_sort_calls(profile_node* l, profile_node* r);
bool operator<=(addr_info l, addr_info r);
