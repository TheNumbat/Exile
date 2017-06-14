
#pragma once

typedef u32 guiid;

struct widget_text {
	f32 point = 0.0f;
	color c;
	string text;
};

struct widget_carrot {
	color c;
	bool active;
};

// TODO(max): this
enum gui_window_style_flags : u16 {
	gui_window_scroll 	= 1<<0,
	gui_window_resize 	= 1<<1,
	gui_window_move   	= 1<<2,
	gui_window_collapse	= 1<<3,
	gui_window_close 	= 1<<4,
};

struct _gui_window {
	bool shown = true;

	string title;
	r2 rect;
	v2 offset, clickoffset;
};

struct gui_opengl {
	context_id 			context;
	shader_program_id 	shader;
	texture_id 			texture;
};

struct gui_style {
	bv3 win_back   = V3b(34, 43, 47);
	bv3 win_top    = V3b(74, 79, 137);
	bv3 win_close  = V3b(102, 105, 185);
	f32 title_padding = 5.0f;
};

struct gui_input_state {
	u16 mousex = 0;
	u16 mousey = 0;
	u16 scroll = 0;

	bool lclick = false;
	bool rclick = false;
	bool mclick = false;
	bool ldbl = false;
};

struct gui_manager {
	vector<_gui_window> windows;	// TODO(max): sort?
	u32 currentwin = 0;
	
	guiid last_id = 1;
	// 0 = invalid, 1 = none
	guiid hot = 1, active = 1;

	mesh_2d mesh;
	
	gui_opengl ogl;
	gui_style  style;
	gui_input_state  input;

	f32 font_point 	= 0.0f;
	asset* font 	= NULL;

	allocator* alloc = NULL;
};

gui_manager make_gui(allocator* alloc, opengl* ogl, asset* font);
void destroy_gui(gui_manager* gui);
guiid getid(gui_manager* gui);

void gui_begin_frame(gui_manager* gui, gui_input_state input);
void gui_end_frame_render(opengl* ogl, gui_manager* gui);

bool gui_window(gui_manager* gui, string title, r2 rect, f32 opacity);
void gui_text_line(gui_manager* gui, string str, f32 point, color c);
void gui_text_line_f(gui_manager* gui, string fmt, f32 point, color c, ...);
bool gui_carrot(gui_manager* gui, color c, bool* toggle);

void gui_render_window(gui_manager* gui, _gui_window* win);
v2 gui_render_widget_text(gui_manager* gui, _gui_window* win, widget_text* text);
v2 gui_render_widget_carrot(gui_manager* gui, _gui_window* win, widget_carrot* carrot);

void push_windowhead(gui_manager* gui, _gui_window* win);
void push_windowbody(gui_manager* gui, _gui_window* win, f32 opacity);
