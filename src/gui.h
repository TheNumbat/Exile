
#pragma once

enum _widget_type {
	widget_none,
	widget_text,
};

struct _widget_text {
	f32 point = 0.0f;
	color c;
	string text;
};

struct gui_widget {
	_widget_type type = widget_none;
	u32 id = 0;
	union {
		_widget_text text;
	};
	gui_widget() : text() {};
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
	u32 id 		= 0;

	string title;
	bool active = true;
	bool resizing = false;
	v2 clickoffset = V2f(0, 0);
	
	r2 rect;
	v2 margin;
	f32 opacity = 1.0f;
	u16 flags  = 0;

	vector<gui_widget> widgets;
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
	v2  win_margin = V2(0.0f, 0.0f);		// depends on font, set on make_gui
	f32 win_minw   = 50.0f;
	f32 win_minh   = 50.0f;
};

struct gui_input {
	_platform_event_mouse mouse;
};

struct gui_manager {
	vector<_gui_window> windows;	// TODO(max): sort?
	u32 current = 0;
	
	u32 active 		= 0;
	u32 hot 		= 0;
	
	mesh_2d mesh;
	
	gui_opengl ogl;
	gui_style  style;
	gui_input  input;

	f32 font_point 	= 0.0f;
	asset* font 	= NULL;

	allocator* alloc = NULL;
};

gui_manager make_gui(allocator* alloc, opengl* ogl, asset* font);
void destroy_gui(gui_manager* gui);

void gui_begin_frame(gui_manager* gui, gui_input input);
void gui_end_frame_render(opengl* ogl, gui_manager* gui);

bool gui_window(u32 ID, gui_manager* gui, string title, r2 rect, f32 opacity);
void gui_text_line(u32 ID, gui_manager* gui, string str, f32 point, color c);
void gui_text_line_f(u32 ID, gui_manager* gui, string fmt, f32 point, color c, ...);

#define gui_window(g,t,r,o) gui_window(__COUNTER__ + 1, g, t, r, o)
#define gui_text_line(g,s,p,c) gui_text_line(__COUNTER__ + 1, g, s, p, c)
#define gui_text_line_f(g,s,p,c,...) gui_text_line_f(__COUNTER__ + 1, g, s, p, c, __VA_ARGS__)

void push_windowhead(gui_manager* gui, _gui_window* win);
void push_windowbody(gui_manager* gui, _gui_window* win);
