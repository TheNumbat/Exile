
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
	horzscroll	= 1<<7,
	autosize	= 1<<8,
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

enum class gui_offset_mode : u8 {
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

enum class win_input_state : u8 {
	none,
	moving,
	resizing,
	scrolling
};

struct gui_window_state {
	r2 rect;
	v2 move_click_offset;

	f32 opacity = 1.0f;
	v2 scroll_pos, previous_content_size;
	u16 flags 	= 0;
	u32 z 		= 1, title_tris = 0; 

	bool active = true;
	bool can_scroll = false;
	win_input_state input = win_input_state::none;

	// TODO(max): these should be style stack parameters
	bool override_active = false;
	bool override_seen = false;

	gui_offset_mode offset_mode = gui_offset_mode::y;
	vector<v2> offset_stack;
	stack<u32> id_hash_stack;
	map<guiid, gui_state_data> state_data;

	v2 current_offset();
	r2 get_real_content();
	r2 get_real_body();
	r2 get_real_top();
	r2 get_real();
	void update_input();
	void clamp_scroll();
	bool seen(r2 rect);
	
	// TODO(max): fix font system
	gui_font* font = null;
	f32 default_point = 14.0f;
	
	// TODO(max): arbitrary # of meshes for images, different fonts
	mesh_2d_col 	shape_mesh;
	mesh_2d_tex_col text_mesh;
};

struct _gui_style {
	f32 font 			= 0.0f;	// default font size 
	f32 title_padding 	= 3.0f;
	f32 line_padding 	= 3.0f;
	u32 log_win_lines 	= 15;
	v2 resize_tab		= V2(20.0f, 20.0f);
	v4 win_margin 		= V4(5.0f, 0.0f, 5.0f, 10.0f); // l t r b
	v2 carrot_padding	= V2(3.0f, 5.0f);
	v2 box_sel_padding	= V2(6.0f, 6.0f);

	f32 default_win_a 	= 0.75f;
	v2 default_win_size = V2f(250, 400);
	v2 min_win_size		= V2f(75, 50);

	v2 default_carrot_size = V2f(10, 10);

	color3 win_back		= V3b(34, 43, 47);
	color3 win_top		= V3b(74, 79, 137);
	color3 win_title 	= V3b(255, 255, 255);
	color3 wid_back		= V3b(102, 105, 185);
	color3 tab_color	= V3b(100, 106, 109);

	f32 indent_size 		= 10.0f;
	f32 win_scroll_w 		= 10.0f;
	f32 win_scroll_speed	= 15.0f;
	color3 win_scroll_back 	= V3b(102, 105, 185);
	color3 win_scroll_bar 	= V3b(132, 135, 215);
};

enum class gui_active_state : u8 {
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

	gui_window_state* current = null;	// we take a pointer into a map but it's OK because we know nothing will be added while this is in use
	u32 last_z = 1;						// this only counts up on window layer changes so we don't have to iterate through
										// the map to check z levels. You'll never get to >2 billion changes, right?
	map<guiid, gui_window_state> window_state_data;

	vector<gui_font> fonts;

	platform_window* window = null;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static gui_manager make(ogl_manager* ogl, allocator* alloc, platform_window* win);
	void destroy();

	// the first font you add is the default size
	void add_font(ogl_manager* ogl, string asset_name, asset_store* store, bool mono = false); 
	void reload_fonts(ogl_manager* ogl);

	gui_window_state* add_window_state_data(guiid id, gui_window_state data);
	gui_state_data* add_state_data(guiid id, gui_state_data data);

	void begin_frame(gui_input_state new_input);
	void end_frame(platform_window* win, ogl_manager* ogl);
};

static gui_manager* ggui; // set at gui_begin_frame, used as context for gui functions
gui_font* gui_select_best_font_scale();

// the functions you call every frame use ggui instead of passing a gui_manager pointer
// (except begin_frame, as this sets up the global pointer)

// These functions you can call from anywhere between starting and ending a frame

void gui_push_offset(v2 offset, gui_offset_mode mode = gui_offset_mode::xy);
void gui_pop_offset();
void gui_set_offset(v2 offset);
v2 	 gui_window_dim();
void gui_indent();
void gui_unindent();
void gui_push_id(u32 id);
void gui_pop_id();

bool gui_occluded();

bool gui_begin(string name, r2 first_size = R2f(40,40,0,0), gui_window_flags flags = 0, f32 first_alpha = 0);
void gui_end();

void gui_text(string text, color c = WHITE, f32 point = 0.0f);
bool gui_node(string text, color c = WHITE, f32 point = 0.0f);
bool gui_carrot_toggle(string name, bool initial = false, bool* toggleme = null);
void gui_slider(string name, i32* val, i32 low, i32 high);

void render_windowhead(gui_window_state* win);
void render_windowbody(gui_window_state* win);
void render_carrot(gui_window_state* win, v2 pos, bool active);

bool operator==(guiid l, guiid r);

