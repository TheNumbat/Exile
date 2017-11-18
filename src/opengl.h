
#pragma once

void ogl_load_global_funcs();

typedef i32 texture_id;

struct shader_source {
	string path;
	platform_file_attributes last_attrib;
	string source;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static shader_source make(string path, allocator* a);
	void load();
	void destroy();
	bool refresh();
};

struct shader_program {
	GLuint handle = 0;
	shader_source vertex;
	shader_source fragment;
	func_ptr<void, shader_program*, render_command*, render_command_list*> set_uniforms;
	// tessellation control, evaluation, geometry

///////////////////////////////////////////////////////////////////////////////

	static shader_program make(string vert, string frag, _FPTR* uniforms, allocator* a);
	void compile();
	bool refresh();
	void destroy();
	bool check_compile(string name, GLuint shader);
};

enum class texture_wrap : u8 {
	repeat,
	mirror,
	clamp,
	clamp_border,
};

struct texture {
	texture_id id 		= 0;
	GLuint handle 		= 0;
	texture_wrap wrap 	= texture_wrap::repeat;
	bool pixelated 		= false;

///////////////////////////////////////////////////////////////////////////////

	static texture make(texture_wrap wrap, bool pixelated);
	void destroy();

	void load_bitmap(asset_store* as, string name);
	void load_bitmap_from_font(asset_store* as, string name);
	void load_bitmap_from_font(asset* font);
};

struct draw_context {
	GLuint vao     = 0;
	GLuint vbos[8] = {};
	func_ptr<void, draw_context*, render_command*> set_buffers;
	func_ptr<void, render_command*> run;
	func_ptr<void, draw_context*> set_attribs;
};

struct ogl_manager {
	map<texture_id, texture> 			   		textures;
	map<render_command_type, shader_program> 	programs;
	map<render_command_type, draw_context> 		contexts;

	texture_id 			next_texture_id = 1;
	
	shader_program dbg_shader;

	allocator* alloc = null;
	string version;
	string renderer;
	string vendor;

///////////////////////////////////////////////////////////////////////////////

	static ogl_manager make(allocator* a);
	void destroy();

	void add_program(render_command_type type, string v, string f, _FPTR* uniforms);
	shader_program* select_program(render_command_type type);
	void try_reload_programs();

	void add_draw_context(render_command_type type, _FPTR* attribs, _FPTR* buffers, _FPTR* run);
	draw_context* select_draw_context(render_command_type type);

	texture_id add_texture(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	texture* select_texture(texture_id id);
	texture_id add_texture_from_font(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	texture_id add_texture_from_font(asset* font, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	void destroy_texture(texture_id id);

	void dbg_render_texture_fullscreen(platform_window* win, texture_id id);
	void execute_command_list(platform_window* win, render_command_list* rcl);

	void cmd_set_settings(platform_window* win, render_command* cmd);
};

void debug_proc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userPointer);
CALLBACK void uniforms_dbg(shader_program* prog, render_command* rc, render_command_list* rcl) {};
