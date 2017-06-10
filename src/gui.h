
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
	v2 pos;
	union {
		_widget_text text;
	};
	gui_widget() : text() {};
};

enum gui_window_flags {
	gui_window_scroll = 1<<0,
	gui_window_resize = 1<<1,
};

struct gui_window {
	string title;
	f32 opacity = 1.0f;
	r2 rect;
	v2 margin;
	u16 flags 	= 0;
	f32 last_y = 0;
	vector<gui_widget> widgets;
};

struct gui_opengl {
	context_id 			context;
	shader_program_id 	shader;
	texture_id 			texture;
};

struct gui_manager {
	gui_window current;
	
	f32 font_point = 0.0f;
	mesh_2d mesh;
	gui_opengl ogl;
	allocator* alloc 	= NULL;
	asset* font 		= NULL;
};

gui_manager make_gui(allocator* alloc, opengl* ogl, asset* font);
void destroy_gui(gui_manager* gui);

void gui_begin_frame(gui_manager* gui);
void gui_end_frame(gui_manager* gui);

void gui_begin_window(gui_manager* gui, string title, r2 rect, f32 opacity = 1.0f);
void gui_end_window(gui_manager* gui);

void gui_text_line(gui_manager* gui, string str, f32 point = 0.0f, color c = V4b(255, 255, 255, 255));
void gui_text_line_f(gui_manager* gui, string fmt, f32 point, color c, ...);

void gui_render(gui_manager* gui, opengl* ogl);
