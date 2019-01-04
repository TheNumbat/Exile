
#pragma once

v2 size_text(asset* font, string text_utf8, f32 point);

typedef i32 texture_id;
typedef i32 gpu_object_id;

struct shader_source {
	string path;
	platform_file_attributes last_attrib;
	string source;
	allocator* alloc = null;

	static shader_source make(string path, allocator* a);
	void load();
	void destroy();
	bool try_refresh();
};

struct render_command; struct render_command_list;

struct shader_program {
	GLuint handle = 0;
	shader_source vertex, geometry, fragment;
	func_ptr<void, shader_program*, render_command*> send_uniforms;
	// tessellation control, evaluation, geometry

	static shader_program make(string vert, string frag, string geom, _FPTR* uniforms, allocator* a);
	void compile();
	bool try_refresh();
	void destroy();
	void gl_destroy();
	void recreate();
	bool check_compile(string name, GLuint shader);
};

enum class texture_wrap : u8 {
	repeat,
	mirror,
	clamp,
	clamp_border,
};

enum class texture_type : u8 {
	none,
	bmp,
	rf,
	array,
	cube
};

struct asset_pair {
	string name;
	asset_store* store = null;
};

struct texture_cube_info {
	asset_pair info;

	void load_single(GLuint handle, asset_store* store, string name);
};

struct texture_array_info {
	iv3 dim;
	i32 current_layer = 0;
	i32 layer_offset = 0;
	array<asset_pair> assets;

	void push(GLuint handle, asset_store* store, string name);
};

struct texture_bmp_info {
	asset_pair info;

	void load(GLuint handle, asset_store* store, string name);
};

struct texture_rf_info {
	asset_pair info;

	void load(GLuint handle, asset_store* store, string name);
};

struct texture {
	texture_type type   = texture_type::none;
	texture_id id 		= 0;
	GLuint handle 		= 0;
	
	gl_tex_target gl_type = gl_tex_target::_2D;
	texture_wrap wrap     = texture_wrap::repeat;
	bool pixelated        = false;
	f32 anisotropy 		  = 1.0f;

	union {	
		texture_rf_info    rf_info;
		texture_bmp_info   bmp_info;
		texture_cube_info  cube_info;
		texture_array_info array_info;
	};

	// WHY C++ WHY
	texture() {}
	texture(texture& t) {_memcpy(&t, this, sizeof(texture));}
	texture(texture&& t) {_memcpy(&t, this, sizeof(texture));}
	texture& operator=(texture& t) {_memcpy(&t, this, sizeof(texture)); return *this;}

	static texture make_cube(texture_wrap wrap, bool pixelated, f32 aniso);
	static texture make_rf(texture_wrap wrap, bool pixelated, f32 aniso);
	static texture make_bmp(texture_wrap wrap, bool pixelated, f32 aniso);
	static texture make_array(iv3 dim, u32 idx_offset, texture_wrap wrap, bool pixelated, f32 aniso, allocator* a);
	void destroy(allocator* a);
	void gl_destroy();

	void recreate();
	void reload_data();
	void set_params();
};

struct ogl_info {
	i32 major = 1, minor = 1;
	string vendor, renderer, version, shader_version;
	vector<string> extensions;

	// some convenience
	i32 max_texture_size = 0, max_texture_layers = 0;
	f32 max_anisotropy = 0.0f;

	static ogl_info make(allocator* a);
	void destroy();

	bool check_version(i32 major, i32 minor);
};

struct gpu_object {
	func_ptr<void, gpu_object*> setup;
	func_ptr<void, gpu_object*, void*, bool> update;

	GLuint vao = 0;
	GLuint vbos[5] = {};

	void* data = null;
	gpu_object_id id = -1;

	static gpu_object make();
	void destroy();
	void recreate();
};

struct draw_context {
	func_ptr<bool, ogl_info*> compat;
	func_ptr<void, render_command*, gpu_object*> run;
	shader_program shader;
};

enum class render_setting : u8 {
	none,
	wireframe,
	depth_test,
	aa_lines,
	blend,
	scissor,
	cull,
	msaa,
	aa_shading,
	write_depth,
	point_size
};

struct cmd_settings {
	bool polygon_line = false;
	bool depth_test = true;
	bool line_smooth = true;
	bool blend = true;
	bool scissor = true;
	bool cull_backface = false;
	bool multisample = true;
	bool sample_shading = false;
	bool depth_mask = true;
	bool point_size = false;
};

struct ogl_settings {
	f32 anisotropy = 1.0f;
};

struct render_command {
	u16 cmd = 0;

	struct {	
		
		texture_id texture0 = -1, texture1 = -1;
		gpu_object_id object = -1;
		void* uniform_info = null;

		m4 model, view, proj;
		u32 sort_key = 0;

		// triangle index, gets * 3 to compute element index
		u32 offset = 0, num_tris = 0, start_tri = 0;

		// zero for entire window
		r2 viewport;
		r2 scissor;

		func_ptr<void, void*> callback;
		void* param = null;
	};

	struct {
		render_setting setting;
		bool enable = false;
	};

	render_command() {}

	static render_command make(u16 type);
	static render_command make(u16 type, gpu_object_id gpu, u32 key = 0);
	static render_command make(u16 type, render_setting setting, bool enable);
};

bool operator<=(render_command& first, render_command& second);

struct render_command_list {
	vector<render_command> commands;
	allocator* alloc = null;

	static render_command_list make(allocator* alloc = null, u32 cmds = 8);
	void destroy();
	void clear();
	void add_command(render_command rc);
	void sort();

	void push_settings();
	void pop_settings();
	void set_setting(render_setting setting, bool enable);
};

enum class camera_mode : u8 {
	first,
	third
};

struct render_camera {

	v3 	  pos, front, up, right;
	float pitch = 0.0f, yaw = 0.0f, fov = 60.0f;

	v3 offset3rd = v3(0, 1, 0);
	f32 reach = 5.0f;

	camera_mode mode = camera_mode::first;

	void update();
	void reset();
	void move(i32 dx, i32 d, f32 sens);
	
	m4 view();
	m4 proj(f32 ar);
	m4 offset();
	m4 view_pos_origin();
	m4 view_no_translate();
};

struct ogl_manager {

	static const u16 cmd_push_settings = 1;
	static const u16 cmd_pop_settings = 2;
	static const u16 cmd_setting = 3;

	map<u16, draw_context> commands;

	map<texture_id, texture> textures;
	map<gpu_object_id, gpu_object> objects;

	ogl_settings 		settings, prev_settings;
	stack<cmd_settings> command_settings;

	shader_program dbg_shader;
	ogl_info info;

	gpu_object_id 	next_gpu_id = 1;
	texture_id 		next_texture_id = 1;
	
	platform_window* win = null;
	allocator* alloc = null;

	static ogl_manager make(platform_window* win, allocator* a);
	void destroy();

	void apply_settings();

	void _cmd_pop_settings();
	void _cmd_push_settings();
	void _cmd_apply_settings();
	void _cmd_set_setting(render_setting setting, bool enable);
	void _cmd_set_settings(render_command* cmd);

	void load_global_funcs();
	void try_reload_programs();
	void check_leaked_handles();
	void reload_texture_assets();
	
	void gl_end_reload();
	void gl_begin_reload();

	draw_context* select_ctx(u16 id);
	void add_command(u16 id, _FPTR* run, _FPTR* uniforms, _FPTR* compat, string v, string f, string g = {});

	texture_id add_cubemap(asset_store* as, string name);
	void push_tex_array(texture_id tex, asset_store* as, string name);
	texture_id begin_tex_array(iv3 dim, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false, u32 offset = 0);
	i32 get_layers(texture_id tex);

	void destroy_texture(texture_id id);
	texture* select_texture(u32 unit, texture_id id);
	void select_textures(render_command* cmd);

	texture_id add_texture(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);
	texture_id add_texture_from_font(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false);

	void destroy_object(gpu_object_id id);
	gpu_object_id add_object(_FPTR* setup, _FPTR* update, void* cpu_data);
	gpu_object* select_object(gpu_object_id id);
	gpu_object* get_object(gpu_object_id id);

	void dbg_render_texture_fullscreen(texture_id id);
	void execute_command_list(render_command_list* rcl);
};

CALLBACK void ogl_apply(void* eng);

CALLBACK void uniforms_dbg(shader_program* prog, render_command* rc, render_command_list* rcl) {};
void debug_proc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userPointer);
