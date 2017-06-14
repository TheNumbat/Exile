
#pragma once

typedef u32 guiid;

enum _widget_type {
	widget_none,
	widget_text,
	widget_carrot,
};

struct _widget_text {
	f32 point = 0.0f;
	color c;
	string text;
};

struct _widget_carrot {
	color c;
	bool active;
};

struct gui_widget {
	_widget_type type = widget_none;
	guiid id = 0;
	union {
		_widget_text text;
		_widget_carrot carrot;
	};
	gui_widget() : text(), carrot() {};
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
	guiid id 		= 0;

	bool active = true;
	bool resizing = false;

	r2 rect;
	v2 margin, offset, clickoffset;

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
};

struct gui_input {
	_platform_event_mouse mouse;
};


struct gui_manager {
	vector<_gui_window> windows;	// TODO(max): sort?
	u32 currentwin = 0;
	
	guiid active = 0;
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

bool gui_window(guiid id, gui_manager* gui, string title, r2 rect, f32 opacity);
void gui_text_line(guiid id, gui_manager* gui, string str, f32 point, color c);
void gui_text_line_f(guiid id, gui_manager* gui, string fmt, f32 point, color c, ...);
bool gui_carrot(guiid id, gui_manager* gui, color c, bool* toggle);

void gui_render_window(gui_manager* gui, _gui_window* win);
v2 gui_render_widget_text(gui_manager* gui, _gui_window* win, gui_widget* text);
v2 gui_render_widget_carrot(gui_manager* gui, _gui_window* win, gui_widget* carrot);

#define gui_window(g,t,r,o) gui_window(__COUNTER__ + 1, g, t, r, o)
#define gui_text_line(g,s,p,c) gui_text_line(__COUNTER__ + 1, g, s, p, c)
#define gui_text_line_f(g,s,p,c,...) gui_text_line_f(__COUNTER__ + 1, g, s, p, c, __VA_ARGS__)
#define gui_carrot(g,c,t) gui_carrot(__COUNTER__ + 1, g, c, t);

void push_windowhead(gui_manager* gui, _gui_window* win);
void push_windowbody(gui_manager* gui, _gui_window* win, f32 opacity);
