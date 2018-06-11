
#pragma once

v2 size_text(asset* font, string text_utf8, f32 point);

struct shader_source {
	string path;
	platform_file_attributes last_attrib;
	string source;
	allocator* alloc = null;

	static shader_source make(string path, allocator* a);
	void load();
	void destroy();
	bool refresh();
};

struct shader_program {
	GLuint handle = 0;
	shader_source vertex;
	shader_source fragment;
	func_ptr<void, shader_program*, render_command*, render_command_list*> send_uniforms;
	// tessellation control, evaluation, geometry

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

struct texture_array_info {
	iv3 dim;
	i32 current_layer = 0;
};

struct texture {
	texture_id id 		= 0;
	GLuint handle 		= 0;
	
	gl_tex_target type 	= gl_tex_target::_2D;
	texture_wrap wrap 	= texture_wrap::repeat;
	bool pixelated 		= false;
	
	texture_array_info array_info;

	static texture make(texture_wrap wrap, bool pixelated);
	static texture make_array(iv3 dim, texture_wrap wrap, bool pixelated);
	void destroy();

	void load_bitmap(asset_store* as, string name);
	void load_bitmap_from_font(asset_store* as, string name);
	void load_bitmap_from_font(asset* font);

	void set_params();
};

struct draw_context {
	func_ptr<void, render_command*> send_buffers;
	func_ptr<void, render_command*> run;
	shader_program shader;
};

struct ogl_info {
	i32 major = 1, minor = 1;
	string vendor, renderer, version, shader_version;
	vector<string> extensions;

	// some convenience
	i32 max_texture_size = 0, max_texture_layers = 0;

	static ogl_info make(allocator* a);
	void destroy();

	bool check_version(i32 major, i32 minor);
};

struct ogl_manager {
	map<texture_id, texture> 			   			textures;
	map<render_command_type_value, draw_context> 	commands;

	shader_program dbg_shader;
	ogl_info info;

	texture_id 				  next_texture_id = 1;
	render_command_type_value next_command_id = 1;
	
	platform_window* win = null;
	allocator* alloc = null;


	static ogl_manager make(platform_window* win, allocator* a);
	void destroy();

	void try_reload_programs();
	void load_global_funcs();
	void check_leaked_handles();

	render_command_type_value add_command(_FPTR* buffers, _FPTR* run, string v, string f, _FPTR* uniforms, _FPTR* compat);
	draw_context* get_command_ctx(render_command_type_value type);

	texture_id begin_tex_array(iv3 dim, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false, u32 offset = 0);
	void push_tex_array(texture_id tex, asset_store* as, string name);

	texture_id add_texture(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	texture* select_texture(texture_id id);
	texture_id add_texture_from_font(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	texture_id add_texture_from_font(asset* font, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	void destroy_texture(texture_id id);

	void dbg_render_texture_fullscreen(texture_id id);
	void execute_command_list(render_command_list* rcl);

	void cmd_set_settings(render_command* cmd);
};

void debug_proc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userPointer);
CALLBACK void uniforms_dbg(shader_program* prog, render_command* rc, render_command_list* rcl) {};
