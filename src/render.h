
#pragma once

v2 size_text(asset* font, string text_utf8, f32 point);

typedef i32 texture_id;

enum class render_command_type : u32 {
	none,
	mesh_2d_col,
	mesh_2d_tex,
	mesh_2d_tex_col,
	mesh_3d_tex
};

struct mesh_2d_col {
	vector<v2>		vertices;	// x y 
	vector<colorf>	colors;		// r g b a
	vector<uv3> 	elements;
	allocator* alloc = null;

	GLuint vao;
	GLuint vbos[3];
	bool dirty = false;

	static mesh_2d_col make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	void clear();

	void push_tri(v2 p1, v2 p2, v2 p3, color c);
	void push_rect(r2 rect, color c);
	void push_cutrect(r2 r, f32 round, color c);
};

struct mesh_2d_tex {
	vector<v2>		vertices;	// x y 
	vector<v2>		texCoords;	// u v
	vector<uv3> 	elements;
	allocator* alloc = null;

	GLuint vao;
	GLuint vbos[3];
	bool dirty = false;

	static mesh_2d_tex make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	void clear();
};

struct mesh_2d_tex_col {
	vector<v2>		vertices;	// x y 
	vector<v2>		texCoords;	// u v
	vector<colorf> 	colors;
	vector<uv3> 	elements;
	allocator* alloc = null;

	GLuint vao;
	GLuint vbos[4];
	bool dirty = false;

	static mesh_2d_tex_col make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	void clear();

	void push_rect(r2 rect, color c);
	void push_cutrect(r2 r, f32 round, color c);
	f32 push_text_line(asset* font, string text_utf8, v2 pos, f32 point = 0.0f, color c = V4b(255, 255, 255, 255)); 
};

struct mesh_3d_tex {
	vector<v3>  vertices;	// x y z
	vector<v2>  texCoords; 	// u v
	vector<uv3> elements;
	allocator* alloc = null;

	GLuint vao;
	GLuint vbos[3];
	bool dirty = false;

	static mesh_3d_tex make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	void clear();

	void push_cube(v3 pos, f32 len);
};

struct render_command {
	render_command_type cmd 	= render_command_type::none;
	texture_id 			texture = -1;
	
	m4 model;
	u32 sort_key = 0;

	// triangle index, gets * 3 to compute element index
	u32 offset = 0, num_tris = 0, start_tri = 0;

	// zero for entire window
	r2 viewport;
	r2 scissor;

	union {
		void* data = null;
		mesh_3d_tex* 	 m3dt;
		mesh_2d_col*	 m2dc;
		mesh_2d_tex* 	 m2dt;
		mesh_2d_tex_col* m2dtc;
	};

	static render_command make(render_command_type type, void* data, u32 key = 0);
};
bool operator<=(render_command& first, render_command& second);

struct render_command_list {
	vector<render_command> commands;
	allocator* alloc = null;
	m4 view;
	m4 proj;

	static render_command_list make(allocator* alloc = null, u32 cmds = 8);
	void destroy();
	void add_command(render_command rc);
	void sort();
};

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

struct texture {
	texture_id id 		= 0;
	GLuint handle 		= 0;
	texture_wrap wrap 	= texture_wrap::repeat;
	bool pixelated 		= false;

	static texture make(texture_wrap wrap, bool pixelated);
	void destroy();

	void load_bitmap(asset_store* as, string name);
	void load_bitmap_from_font(asset_store* as, string name);
	void load_bitmap_from_font(asset* font);
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
};

struct ogl_manager {
	map<texture_id, texture> 			   		textures;
	map<render_command_type, draw_context> 		commands;

	shader_program dbg_shader;
	ogl_info info;

	texture_id 			next_texture_id = 1;
	
	allocator* alloc = null;

	static ogl_manager make(allocator* a);
	void destroy();

	void try_reload_programs();
	void check_version_and_extensions();
	void load_global_funcs();
	void check_leaked_handles();

	void add_command_ctx(render_command_type type, _FPTR* buffers, _FPTR* run, string v, string f, _FPTR* uniforms);
	draw_context* get_command_ctx(render_command_type type);

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
