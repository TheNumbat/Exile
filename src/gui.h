
#pragma once

typedef u16 gui_window_flags;

struct guiid {
	u32 base;
	string name;
};

enum _gui_window_flags : gui_window_flags {
	win_noresize 	= 1<<0,
	win_nomove		= 1<<1,
	win_nohide		= 1<<2,
	win_noinput 	= win_noresize | win_nomove | win_nohide,
};

struct gui_opengl {
	context_id			context;
	shader_program_id 	shader;
	texture_id 			texture;
};

struct gui_input_state {
	v2 mousepos;
	u16 scroll = 0;

	bool lclick = false;
	bool rclick = false;
	bool mclick = false;
	bool ldbl = false;
};

union gui_state_data {
	struct {
		u16 u16_1, u16_2, u16_3, u16_4;
	};
	struct {
		i16 i16_1, i16_2, i16_3, i16_4;
	};
	struct {
		u32 u32_1, u32_2;
	};
	struct {
		i32 i32_1, i32_2;
	};
	struct {
		f32 f32_1, f32_2;
	};
	u64 u64_1;
	i64 i64_1;
	f64 f64_1;
	bool b;
	void* data = NULL;
};

struct gui_window_state {
	r2 rect;
	v2 title_size;
	v2 move_click_offset;

	f32 opacity = 1.0f;
	u16 flags 	= 0;
	
	bool active = true;
	bool resizing = false;

	stack<v2> offset_stack;
	stack<u32> id_hash_stack;
	mesh_2d mesh;
};

struct gui_style {
	f32 gscale 			= 1.0f;	// global scale 
	f32 font 			= 0.0f;	
	f32 title_padding 	= 5.0f;
	f32 line_padding 	= 3.0f;
	v2 carrot_padding	= V2(10.0f, 5.0f);

	f32 default_win_a 	= 0.75f;
	v2 default_win_size = V2f(250, 400);
	v2 min_win_size		= V2f(75, 50);

	v2 default_carrot_size = V2f(10, 10);

	color3 win_back		= V3b(34, 43, 47);
	color3 win_top		= V3b(74, 79, 137);
	color3 win_close	= V3b(102, 105, 185);
	color3 win_title 	= V3b(255, 255, 255);
};

struct gui_manager {

	// TODO(max): this will break on hash collision. The IDs are otherwise abstracted
	// through the guiid maps, but not here. 
	i64 hash_active; // -2 = invalid, -1 = none

	gui_style 		style;
	gui_input_state input;
	gui_opengl 		ogl;

	gui_window_state* current = NULL;
	map<guiid, gui_window_state> 	window_state_data;
	map<guiid, gui_state_data> 		state_data;

	asset* font = NULL;
	allocator* alloc = NULL;
};

static gui_manager* ggui;

gui_manager make_gui(asset* font, opengl* ogl, allocator* alloc);
void destroy_gui(gui_manager* gui);

void gui_begin_frame(gui_manager* gui, gui_input_state input);
void gui_end_frame(opengl* ogl);

bool gui_begin(string name, r2 first_size = R2f(40,40,0,0), f32 first_alpha = 0, gui_window_flags flags = 0);
bool gui_carrot_toggle(string name, bool initial = false, color c = V4b(255, 255, 255, 255),  v2 pos = V2f(0,0), bool* toggleme = NULL);

void push_windowhead(gui_window_state* win);
void push_windowbody(gui_window_state* win);
void push_text(gui_window_state* win, v2 pos, string text, f32 point, color c);
void push_carrot(gui_window_state* win, v2 pos, bool active, color c);

bool operator==(guiid l, guiid r);
