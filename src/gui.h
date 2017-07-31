
#pragma once

typedef u16 gui_window_flags;

struct guiid {
	u32 base = 0;
	string name;
};

enum class window_flags : gui_window_flags {
	noresize 	= 1<<0,
	nomove		= 1<<1,
	nohide		= 1<<2,
	noscroll	= 1<<3,
	noinput 	= noresize | nomove | nohide | noscroll,
	nowininput	= noresize | nomove | nohide,
	nohead		= 1<<4 | nohide | nomove,
	noback		= 1<<5 | noresize,
	ignorescale = 1<<6,
};

struct gui_opengl {
	context_id			context = 0;
	shader_program_id 	shader 	= 0;
	texture_id			font 	= 0;
};

struct gui_input_state {
	v2 mousepos;
	i16 scroll = 0;

	bool lclick = false;
	bool rclick = false;
	bool mclick = false;
	bool ldbl = false;
};

// whatever we want to store in 64 bits
union gui_state_data {
	struct {
		u16 u16_1;
		u16 u16_2;
		u16 u16_3;
		u16 u16_4;
	};
	struct {
		i16 i16_1;
		i16 i16_2;
		i16 i16_3;
		i16 i16_4;
	};
	struct {
		u32 u32_1;
		u32 u32_2;
	};
	struct {
		i32 i32_1;
		i32 i32_2;
	};
	struct {
		f32 f32_1;
		f32 f32_2;
	};
	u64 u64_1;
	i64 i64_1;
	f64 f64_1;
	bool b;
	void* data = null;
};

enum class gui_offset_mode {
	xy,
	x,
	y,
};

struct gui_font {
	asset_store* store = null;
	string asset_name;

	bool mono = false;
	asset* font = null;
	texture_id texture = 0;
};

struct gui_window_state {
	r2 rect;
	v2 move_click_offset;

	f32 opacity = 1.0f;
	u16 flags 	= 0;
	u32 z 		= 0; 
	
	bool active = true;
	bool resizing = false;

	gui_offset_mode offset_mode = gui_offset_mode::y;
	vector<v2> offset_stack;
	stack<u32> id_hash_stack;

	bool mono = false;
	gui_font* font = null;
	mesh_2d mesh;
};

struct _gui_style {
	f32 gscale 			= 1.0f;	// global scale 
	f32 font 			= 0.0f;	// default font size - may use different actual font based on gscale * font
	f32 title_padding 	= 5.0f;
	f32 line_padding 	= 3.0f;
	u32 log_win_lines 	= 15;
	f32 resize_tab		= 0.075f;
	v4 win_margin 		= V4(15.0f, 15.0f, 10.0f, 15.0f); // l r t b
	v2 carrot_padding	= V2(10.0f, 5.0f);
	v2 box_sel_padding	= V2(6.0f, 6.0f);

	f32 default_win_a 	= 0.75f;
	v2 default_win_size = V2f(250, 400);
	v2 min_win_size		= V2f(75, 50);

	v2 default_carrot_size = V2f(10, 10);

	color3 win_back		= V3b(34, 43, 47);
	color3 win_top		= V3b(74, 79, 137);
	color3 win_title 	= V3b(255, 255, 255);
	color3 wid_back		= V3b(102, 105, 185);

	f32 win_scroll_w 		= 15.0f;
	f32 win_scroll_margin	= 2.0f;
	f32 win_scroll_bar_h	= 25.0f;
	color3 win_scroll_back 	= V3b(102, 105, 185);
	color3 win_scroll_bar 	= V3b(132, 135, 215);
};

enum class gui_active_state {
	active,
	none,
	invalid,
	captured,
};

struct gui_manager {

	guiid active_id;
	gui_active_state active = gui_active_state::none;
	_gui_style style;

	gui_input_state input;
	gui_opengl 		ogl;

	gui_window_state* current = null;	// we take a pointer into a map but it's OK because we know nothing will be added while this is in use
	u32 last_z = 0;						// this only counts up on window layer changes so we don't have
										// to iterate through the map to check z levels. You'll never 
										// get to >4 billion changes, right?
	map<guiid, gui_window_state> 	window_state_data;
	map<guiid, gui_state_data> 		state_data;

	vector<gui_font> fonts;

	arena_allocator scratch; // reset whenever (on whatever thread) (currently on formatting log messages)
	allocator* alloc = null;
};

static gui_manager* ggui;

// the functions you call every frame use ggui instead of passing a gui_manager pointer
// (except begin_frame, as this sets up the global pointer)

gui_manager make_gui(ogl_manager* ogl, allocator* alloc);
void destroy_gui(gui_manager* gui);

// call these before or after a frame
void gui_add_font(ogl_manager* ogl, gui_manager* gui, string asset_name, asset_store* store, bool mono = false); // the first font you add is the default size
void gui_reload_fonts(ogl_manager* ogl, gui_manager* gui);

void gui_begin_frame(gui_manager* gui, gui_input_state input);
void gui_end_frame(ogl_manager* ogl);
gui_font* gui_select_best_font_scale();

void gui_push_offset(v2 offset);
void gui_pop_offset();
void gui_set_offset_mode(gui_offset_mode mode = gui_offset_mode::y);

bool gui_occluded();
bool gui_begin(string name, r2 first_size = R2f(40,40,0,0), f32 first_alpha = 0, gui_window_flags flags = 0, bool mono = false);
bool gui_carrot_toggle(string name, bool initial = false, v2 pos = V2f(0,0), bool* toggleme = null);
void gui_box_select(i32* selected, i32 num, v2 pos, ...);

void gui_log_wnd(string name, vector<log_message>* cache);

// these take into account only gscale & win_ignorescale - window + offset transforms occur in gui_ functions
void push_windowhead(gui_window_state* win);
void push_windowbody(gui_window_state* win);
void push_text(gui_window_state* win, v2 pos, string text, f32 point, color c);
void push_carrot(gui_window_state* win, v2 pos, bool active);

bool operator==(guiid l, guiid r);
