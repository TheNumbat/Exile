
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
	horzscroll	= 1<<7,										// TODO(max)
	autosize	= 1<<8,										// TODO(max)
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

enum class gui_cursor_mode : u8 {
	none,
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

struct gui_window {
	r2 rect;
	v2 cursor, last_offset, header_size;

	v2  click_offset, scroll_pos, previous_content_size;
	f32 opacity = 1.0f;
	u16 flags 	= 0;
	u32 z 		= 1, indent_level = 0;
	u32 title_verts = 0, title_elements = 0; 

	bool active = true;
	bool can_scroll = false;
	win_input_state input = win_input_state::none;
	gui_cursor_mode cursor_mode = gui_cursor_mode::y;

	stack<u32> id_hash_stack;
	map<guiid, gui_state_data> state_data;

	// TODO(max): fix font system
	gui_font* font = null;
	allocator* alloc = null;

	// TODO(max): arbitrary # of meshes for images, different fonts
	mesh_2d_col 	background_mesh, shape_mesh;
	mesh_2d_tex_col text_mesh;

	static gui_window make(r2 first, f32 first_alpha, u16 flags, allocator* alloc);
	void destroy();
	void reset();

	r2 get_real_content();
	r2 get_real_body();
	r2 get_real_top();
	r2 get_real();
	r2 get_title();
	void update_input();
	void clamp_scroll();
	bool visible(r2 rect);
	gui_state_data* add_state(guiid id, gui_state_data state);
};

struct _gui_style {
	f32 font_size		= 14.0f;	// default font size 
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

	v2 carrot_size 	 	= V2f(10, 10);
	f32 slider_w 		= 150.0f;

	color3 win_back		= V3b(34, 43, 47);
	color3 win_top		= V3b(74, 79, 137);
	color3 wid_back		= V3b(102, 105, 185);
	color3 tab_color	= V3b(100, 106, 109);

	f32 indent_size 		= 10.0f;
	f32 win_scroll_w 		= 10.0f;
	f32 win_scroll_speed	= 15.0f;
	f32 scroll_slop			= 3.0f;
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

	gui_window* current = null;	// we take a pointer into a map but it's OK because we know nothing will be added while this is in use
	u32 last_z = 1;						// this only counts up on window layer changes so we don't have to iterate through
										// the map to check z levels. You'll never get to >2 billion changes, right?
	map<guiid, gui_window> windows;

	vector<gui_font> fonts;

	platform_window* window = null;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static gui_manager make(ogl_manager* ogl, allocator* alloc, platform_window* win);
	void destroy();

	// the first font you add is the default size
	void add_font(ogl_manager* ogl, string asset_name, asset_store* store, bool mono = false); 
	void reload_fonts(ogl_manager* ogl);

	gui_window* add_window(guiid id, gui_window data);
	gui_state_data* add_state_data(guiid id, gui_state_data data);

	void begin_frame(gui_input_state new_input);
	void end_frame(platform_window* win, ogl_manager* ogl);
};

static gui_manager* ggui; // set at gui_begin_frame, used as context for gui functions
gui_font* gui_select_best_font_scale();

// the functions you call every frame use ggui instead of passing a gui_manager pointer
// (except begin_frame, as this sets up the global pointer)

// These functions you can call from anywhere between starting and ending a frame

void gui_add_offset(v2 offset, gui_cursor_mode override_mode = gui_cursor_mode::none);
void gui_set_offset(v2 offset);
v2 	 gui_window_dim();

void gui_indent();
void gui_unindent();
u32  gui_indent_level();
void gui_same_line();
void gui_left_cursor();
bool gui_in_win();

void gui_push_id(u32 id);
void gui_pop_id();

bool gui_begin(string name, r2 first = R2f(40,40,0,0), gui_window_flags flags = 0, f32 first_alpha = 0);
void gui_end();

void gui_text(string text);
bool gui_node(string text, bool* store = null);
bool gui_carrot_toggle(string name, bool initial = false, bool* toggleme = null);
bool gui_button(string text);

bool gui_checkbox(string name, bool* data);
i32  gui_int_slider(string text, i32* data, i32 low, i32 high);

template<typename V>
void gui_combo(string name, map<string,V> options, V* data);

template<typename enumer>
void gui_enum_buttons(string name, enumer* val);

void render_title(gui_window* win, v2 pos, string title);
void render_windowhead(gui_window* win);
void render_windowbody(gui_window* win);
void render_carrot(gui_window* win, v2 pos, bool active);
void render_checkbox(gui_window* win, r2 pos, bool active);
void render_slider(gui_window* win, r2 rect, i32 rel, i32 max);

void _carrot_toggle_background(bool* data);

bool operator==(guiid l, guiid r);

