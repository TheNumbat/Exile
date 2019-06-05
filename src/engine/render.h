
#pragma once

v2 size_text(asset* font, string text_utf8, f32 point);

typedef i32 texture_id;
typedef i32 draw_cmd_id;
typedef i32 gpu_object_id;
typedef i32 framebuffer_id;

struct shader_source {

	friend struct shader_program;
	friend struct ogl_manager;
	friend void make_meta_info();

private: 

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

	friend struct ogl_manager;
	friend void make_meta_info();

	i32 location(string name);

private:

	GLuint handle = 0;
	shader_source vertex, geometry, fragment;
	func_ptr<void, shader_program*, render_command*> send_uniforms;
	// tessellation control, evaluation, geometry

	static shader_program make(string vert, string frag, string geom, _FPTR* uniforms, allocator* a);
	void destroy();
	void gl_destroy();
	void recreate();

	void compile();
	bool try_refresh();
	bool check_compile(string name, GLuint shader);
	
	void bind();
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
	cube,
	target
};

struct asset_pair {
	string name;
	asset_store* store = null;
};

struct texture_cube_info {
	asset_pair info;
	iv2 dim;

	void load_single(GLuint handle, asset_store* store, string name, bool srgb);
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
	iv2 dim;

	void load(GLuint handle, asset_store* store, string name, bool srgb);
};

struct texture_rf_info {
	asset_pair info;
	iv2 dim;

	void load(GLuint handle, asset_store* store, string name, bool srgb);
};

struct texture_target_info {
	gl_tex_format format = gl_tex_format::rgba8;
	i32 samples = 1;
	iv2 dim;
};

struct texture {

	friend struct ogl_manager;
	friend struct render_target;
	friend struct framebuffer;
	friend void make_meta_info();

	texture() {}
	texture(texture& t) {_memcpy(&t, this, sizeof(texture));}
	texture(texture&& t) {_memcpy(&t, this, sizeof(texture));}
	texture& operator=(texture& t) {_memcpy(&t, this, sizeof(texture)); return *this;}

private:

	texture_type type   = texture_type::none;
	texture_id id 		= 0;
	GLuint handle 		= 0;
	
	gl_tex_target gl_type = gl_tex_target::_2D;
	texture_wrap wrap     = texture_wrap::repeat;
	bool pixelated        = false;
	bool srgb 			  = false;
	f32 anisotropy 		  = 1.0f;

	union {	
		texture_rf_info     rf_info;
		texture_bmp_info    bmp_info;
		texture_cube_info   cube_info;
		texture_array_info  array_info;
		texture_target_info target_info;
	};

	static texture make_cube(texture_wrap wrap, bool pixelated, bool srgb, f32 aniso);
	static texture make_rf(texture_wrap wrap, bool pixelated, bool srgb, f32 aniso);
	static texture make_bmp(texture_wrap wrap, bool pixelated, bool srgb, f32 aniso);
	static texture make_array(iv3 dim, u32 idx_offset, texture_wrap wrap, bool pixelated, bool srgb, f32 aniso, allocator* a);
	static texture make_target(iv2 dim, i32 samples, gl_tex_format format, bool pixelated);
	void destroy(allocator* a);
	void gl_destroy();

	void recreate();
	void reload_data();
	void set_params();

	void bind(u32 unit = 0);
};

enum class render_target_type : u8 {
	tex,
	buf
};

struct render_buffer {

	friend struct render_target;
	friend struct framebuffer;
	friend void make_meta_info();

	static render_buffer make(gl_tex_format format, iv2 dim, i32 samples = 1);

private:

	GLuint handle = 0;
	gl_tex_format format = gl_tex_format::rgba8;
	i32 samples = 1;
	iv2 dim;
	
	void gl_destroy();
	void recreate();
	void bind();
};

struct render_target {

	friend struct framebuffer;
	friend struct ogl_manager;
	friend void make_meta_info();

	render_target() {}
	render_target(render_target& t) {_memcpy(&t, this, sizeof(render_target));}
	render_target(render_target&& t) {_memcpy(&t, this, sizeof(render_target));}
	render_target& operator=(const render_target& t) {_memcpy((void*)&t, this, sizeof(render_target)); return *this;}

private:

	render_target_type type;
	gl_draw_target target = gl_draw_target::color_0;
	union {
		render_buffer* buffer;
		texture* tex;
	};

	static render_target make_tex(gl_draw_target target, texture* tex);
	static render_target make_buf(gl_draw_target target, render_buffer* buf);

	void gl_destroy();
	void recreate();
	void bind();
};

struct framebuffer {

	friend struct ogl_manager;
	friend void make_meta_info();

private:

	GLuint handle = 0;
	vector<render_target> targets;

	static framebuffer make(allocator* a);
	void destroy();
	void gl_destroy();
	void recreate();

	void add_target(render_target target);
	void commit();

	void bind();
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

	friend struct ogl_manager;
	friend void make_meta_info();

	GLuint vao = 0;
	GLuint vbos[5] = {};
	
	func_ptr<void, gpu_object*, void*, bool> update;
	void* data = null;

private:

	func_ptr<void, gpu_object*> setup;
	gpu_object_id id = -1;

	static gpu_object make();
	void destroy();
	void recreate();
};

struct draw_context {

	friend struct ogl_manager;
	friend void make_meta_info();

private:

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
	
	draw_cmd_id cmd_id = -1;
	framebuffer_id fb_id = -1;
	gpu_object_id obj_id = -1;
	
	texture_id textures[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

	void* user_data0 = null;
	void* user_data1 = null;
	u32 sort_key = 0;

	m4 model, view, proj;
	r2 viewport;
	r2 scissor;

	// triangle index, gets * 3 to compute element index
	u32 offset = 0, num_tris = 0, start_tri = 0;

	void* callback_data = null;
	func_ptr<void, void*> callback;

	bool enable = false;
	render_setting setting;

	colorf clear_color;
	GLbitfield clear_components = 0;

	render_command() {}

	static render_command make(draw_cmd_id type);
	static render_command make(draw_cmd_id type, gpu_object_id gpu, u32 key = 0);
	static render_command make(draw_cmd_id type, render_setting setting, bool enable);
};

bool operator<=(render_command& first, render_command& second);

struct render_command_list {
	vector<render_command> commands;

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

	static const draw_cmd_id cmd_push_settings = 0;
	static const draw_cmd_id cmd_pop_settings  = 1;
	static const draw_cmd_id cmd_setting       = 2;
	static const draw_cmd_id cmd_clear         = 3;
	
	ogl_info info;
	ogl_settings settings;

	// Management
	static ogl_manager make(platform_window* win, allocator* a);
	void destroy();

	void gl_end_reload();
	void gl_begin_reload();
	void apply_settings();
	void try_reload_programs();
	void load_global_funcs();
	void reload_texture_assets();

	// Commands
	draw_cmd_id add_command(_FPTR* run, _FPTR* uniforms, _FPTR* compat, string v, string f, string g = {});
	void rem_command(draw_cmd_id id);

 	// Textures
	texture_id add_cubemap(asset_store* as, string name, bool srgb);
 	texture_id add_texture_target(iv2 dim, i32 samples, gl_tex_format format, bool pixelated = true);
	texture_id add_texture(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false, bool srgb = true);
	texture_id add_texture_from_font(asset_store* as, string name, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false, bool srgb = false);

	texture_id begin_tex_array(iv3 dim, texture_wrap wrap = texture_wrap::repeat, bool pixelated = false, bool srgb = true, u32 offset = 0);
	void push_tex_array(texture_id tex, asset_store* as, string name);
	i32 get_layers(texture_id tex);

 	void destroy_texture(texture_id id);

 	// Objects
	gpu_object_id add_object(_FPTR* setup, _FPTR* update, void* cpu_data);
 	void destroy_object(gpu_object_id id);

 	void object_trigger_update(gpu_object_id id, void* data, bool force);

 	// Framebuffers
 	framebuffer_id add_framebuffer();
 	
 	void commit_framebuffer(framebuffer_id id);
 	void add_target(framebuffer_id id, render_target target);
 	
 	void destroy_framebuffer(framebuffer_id id);

	render_target make_target(gl_draw_target target, texture_id tex);
	render_target make_target(gl_draw_target target, render_buffer* buf);

 	// Rendering
 	void dbg_render_texture_fullscreen(texture_id id);
	void execute_command_list(render_command_list* rcl);

	friend void make_meta_info();

private:

	map<texture_id, texture> textures;
	map<gpu_object_id, gpu_object> objects; // TODO(max): there are a few orders of magnitude more of these than
											// the other things, do we really want to put them all in one map?
	map<draw_cmd_id, draw_context> commands;
	map<framebuffer_id, framebuffer> framebuffers;

	ogl_settings 		prev_settings;
	stack<cmd_settings> command_settings;

	shader_program dbg_shader;

	gpu_object_id 	next_gpu_id = 1;
	texture_id 		next_texture_id = 1;
	draw_cmd_id 	next_draw_cmd_id = 4;
	framebuffer_id  next_framebuffer_id = 1;

	platform_window* win = null;
	allocator* alloc = null;

	void _cmd_pop_settings();
	void _cmd_push_settings();
	void _cmd_apply_settings();
	void _cmd_set_setting(render_setting setting, bool enable);
	void _cmd_set_settings(render_command* cmd);
	void _cmd_clear(colorf clear_color, GLenum components);

	void check_leaked_handles();
	
	draw_context* select_ctx(draw_cmd_id id);

	texture* select_texture(u32 unit, texture_id id);
	texture* get_texture(texture_id id);
	void select_textures(render_command* cmd);
	
	gpu_object* select_object(gpu_object_id id);
	gpu_object* get_object(gpu_object_id id);

	framebuffer* select_framebuffer(framebuffer_id id);
};

CALLBACK void ogl_apply(void* eng);
CALLBACK void uniforms_dbg(shader_program* prog, render_command* rc, render_command_list* rcl) {};
void debug_proc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userPointer);
