
#pragma once

#include <engine/basic.h>
#include <engine/ds/vector.h>
#include <engine/render.h>

struct chunk_quad {

	u8  z_1, x_1, z_0, x_0;
	u8  z_3, x_3, z_2, x_2;
	u32 uy01;
	u32 vy23;

	u16 aol, t;
	u8 l3, l2, l1, l0;
	u8 s3, s2, s1, s0;
};
// TODO(max): 32 byte alignment might be slightly faster
static_assert(sizeof(chunk_quad) == 28, "chunk_quad size != 28");

struct mesh_chunk {

	vector<chunk_quad> quads;

	gpu_object_id gpu = -1;
	bool dirty = false;

	static mesh_chunk make_cpu(u32 verts = 4096, allocator* alloc = null);
	void init_gpu();
	void destroy();
	void free_cpu();
	void clear();
	void swap_mesh(mesh_chunk other);

	void quad(iv3 v_0, iv3 v_1, iv3 v_2, iv3 v_3, iv2 uv, i32 t, u8 ql, bv4 a0, bv4 l);
};

// NOTE(max): only ever need one of these to exist
struct mesh_cubemap {
	const v3 vertices[36] = {
		{-1.0f,  1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{-1.0f, -1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f,  1.0f},
		{-1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{-1.0f, -1.0f,  1.0f},
		{-1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f}
	};

	gpu_object_id gpu = -1;

	void init();
	void destroy();
};

struct light_data {
	v3 pos;
	v3 col;
};

struct mesh_light_list {
	vector<light_data> lights;
	
	const f32 t = (1.0f + sqrtf(5.0f)) / 2.0f;
	static const i32 nelems = 60;
	const v3 verts[12] = {
		{-1.0f,  t,  0.0f},
		{ 1.0f,  t,  0.0f},
		{-1.0f, -t,  0.0f},
		{ 1.0f, -t,  0.0f},
		{ 0.0f, -1.0f,  t},
		{ 0.0f,  1.0f,  t},
		{ 0.0f, -1.0f, -t},
		{ 0.0f,  1.0f, -t},
		{ t,  0.0f, -1.0f},
		{ t,  0.0f,  1.0f},
		{-t,  0.0f, -1.0f},
		{-t,  0.0f,  1.0f}
	};
	const uv3 elems[nelems/3] = {
		{0, 11, 5},
		{0, 5, 1},
		{0, 1, 7},
		{0, 7, 10},
		{0, 10, 11},
		{1, 5, 9},
		{5, 11, 4},
		{11, 10, 2},
		{10, 7, 6},
		{7, 1, 8},
		{3, 9, 4},
		{3, 4, 2},
		{3, 2, 6},
		{3, 6, 8},
		{3, 8, 9},
		{4, 9, 5},
		{2, 4, 11},
		{6, 2, 10},
		{8, 6, 7},
		{9, 8, 1}
	};

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();
	void clear();
	void push(v3 pos, v3 col);
};
CALLBACK void setup_mesh_light_list(gpu_object* obj);
CALLBACK void update_mesh_light_list(gpu_object* obj, void* data, bool force);

struct mesh_quad {
	const v4 vbo_data[6] = {
	    {-1.0f,  1.0f,  0.0f, 1.0f},
	    {-1.0f, -1.0f,  0.0f, 0.0f},
	    { 1.0f, -1.0f,  1.0f, 0.0f},

	    {-1.0f,  1.0f,  0.0f, 1.0f},
	    { 1.0f, -1.0f,  1.0f, 0.0f},
	    { 1.0f,  1.0f,  1.0f, 1.0f}
	};

	gpu_object_id gpu = -1;

	void init();
	void destroy();
};

struct mesh_lines {
	vector<v3> 	   vertices;
	vector<colorf> colors;
	
	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();
	void clear();

	void push(v3 p1, v3 p2, colorf c1, colorf c2);
	void push(v3 p1, v3 p2, colorf c);
};

struct mesh_pointcloud {
	vector<v4> vertices;
	
	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();

	void push_points(v3 center, f32 r, i32 divisions, f32 jitter);
	void push(v3 p, f32 s);
};

struct mesh_2d_col {
	vector<v2>		vertices;	// x y 
	vector<colorf>	colors;		// r g b a
	vector<uv3> 	elements;

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();

	void push_tri(v2 p1, v2 p2, v2 p3, color c);
	void push_rect(r2 rect, color c);
	void push_cutrect(r2 r, f32 round, color c);
};
 
struct mesh_2d_tex {
	vector<v2>		vertices;	// x y 
	vector<v2>		texCoords;	// u v
	vector<uv3> 	elements;

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();
};

struct mesh_2d_tex_col {
	vector<v2>		vertices;	// x y 
	vector<v2>		texCoords;	// u v
	vector<colorf> 	colors;
	vector<uv3> 	elements;

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();

	void push_rect(r2 rect, color c);
	void push_cutrect(r2 r, f32 round, color c);
	f32 push_text_line(asset* font, string text_utf8, v2 pos, f32 point = 0.0f, color c = color(255, 255, 255, 255)); 
};

struct mesh_3d_tex {
	vector<v3>  vertices;	// x y z
	vector<v2>  texCoords; 	// u v
	vector<uv3> elements;

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();

	void push_cube(v3 pos, f32 len);
	void push_dome(v3 center, f32 r, i32 divisions);
};

struct mesh_3d_tex_instance_data {
	
	mesh_3d_tex* parent = null;
	vector<v3> data;

	u32 instances = 0;

	gpu_object_id gpu = -1;
	bool dirty = false;

	void init(mesh_3d_tex* parent, u32 instances = 32, allocator* alloc = null);
	void destroy();
	void clear();
	bool empty();
};

struct world;
struct chunk;
struct world_time;

enum class exile_component_view : i32 {
	none = 0,
	albedo,
	pos,
	norm,
	all_light,
	torch,
	sun,
	ao,
	dynamic,
	depth,
	light_volume
};

struct render_settings {
	
	float ambient_factor = 0.005f;
	v4 ao_curve = v4(0.6f, 0.7f, 0.8f, 1.0f);

	bool wireframe = false;
	bool cull_backface = true;
	bool sample_shading = true;
	
	bool dist_fog = false;
	bool block_light = true;
	bool smooth_light = true;
	bool dynamic_light = true;
	bool ambient_occlusion = true;

	exile_component_view view =  exile_component_view::none;

	f32 light_radius = 2.0f;

	i32 num_samples = 4;
	f32 gamma = 2.1f;
	bool enable_gamma = true;
	bool invert_effect = false;
};

struct world_buffers {
	texture_id col_buf, norm_buf, light_buf, depth_buf;
	render_target col_buf_target, depth_buf_target, stencil_buf_target;
	render_target norm_buf_target, light_buf_target;
	
	framebuffer_id chunk_target = 0;
	framebuffer_id light_target = 0;
};
struct effect_buffers {
	texture_id effect0, effect1;
	render_target effect0_target, effect1_target; 
	framebuffer_id effect0_fb = 0;
	framebuffer_id effect1_fb = 0;
};

struct world_proj_info {
	m4 ivp, vp;
	f32 near;
};

struct world_target_info {

	// NOTE(max): only the buffers + textures use GPU memory,
	// the rest are reference objects

	world_buffers  w;
	effect_buffers e;

	bool msaa = false;
	bool current0 = true;

	void init(iv2 dim, i32 samples);
	void destroy();
	
	void flip_fb();
	framebuffer_id world_fb();
	framebuffer_id get_fb();
	texture_id get_output();
};

struct effect_pass {

	draw_cmd_id cmd_id = -1;

	void init(_FPTR* uniforms, string frag);
	void init(_FPTR* uniforms, string vert, string frag);
	void destroy();

	render_command make_cmd();
	void effect(render_command_list* list);
};
CALLBACK void run_effect(render_command* cmd, gpu_object* gpu);		
CALLBACK void setup_mesh_quad(gpu_object* obj);
CALLBACK void update_mesh_quad(gpu_object* obj, void* data, bool force);

// NOTE(max): sort of a client implementation using the engine OGL renderer 
struct exile_renderer {

	allocator* alloc = null;

	void init(allocator* a);
	void destroy();

	draw_cmd_id cmd_2D_col           = 0, cmd_2D_tex      = 0,
                cmd_2D_tex_col       = 0, cmd_3D_tex      = 0,
                cmd_3D_tex_instanced = 0, cmd_lines       = 0;

	draw_cmd_id cmd_pointcloud       = 0, cmd_cubemap       = 0,
                cmd_chunk            = 0, cmd_skydome       = 0,
                cmd_skyfar           = 0, cmd_defer_light   = 0,
                cmd_defer_light_ms	 = 0, cmd_defer_stencil = 0;

	render_settings settings;

	// NOTE(max): these should be static, but hot reloading
	mesh_cubemap the_cubemap;
	mesh_quad 	 the_quad;
	mesh_light_list lights;

	effect_pass invert, gamma;
	effect_pass composite, composite_resolve, resolve;
	effect_pass comp_resolve_light, comp_light;

	world_target_info world_target;

	// TODO(max): combine to one list and sort
	render_command_list world_tasks, hud_tasks;
	world_proj_info proj_info;

	void hud_2D(gpu_object_id gpu_id);

	void world_clear();
	void world_lines(gpu_object_id id, m4 view, m4 proj);
	void world_stars(gpu_object_id gpu_id, world_time* time, m4 view, m4 proj);
	void world_skydome(gpu_object_id gpu_id, world_time* time, texture_id sky, m4 view, m4 proj);
	
	void world_begin_chunks(world* w, bool offset);
	// NOTE(max): this assumes the chunk mesh object is long-lived
	void world_chunk(chunk* c, texture_id blocks, texture_id sky, m4 model, m4 view, m4 proj);
	void world_finish_chunks();

	void resolve_lighting();

	void generate_commands();
	void generate_targets();
	void recreate_targets();
	void check_recreate();

	void calculate_light_quad(m4 m, m4 vp, v3 pos, v3 col);

	iv2 prev_dim;
	i32 prev_samples = 0;

	void end_frame();
};

#define decl_mesh(name) \
	CALLBACK void uniforms_mesh_##name(shader_program* prog, render_command* cmd); 	\
	CALLBACK void setup_mesh_##name(gpu_object* obj); 								\
	CALLBACK void update_mesh_##name(gpu_object* obj, void* data, bool force);		\
	CALLBACK void run_mesh_##name(render_command* cmd, gpu_object* gpu);

decl_mesh(skyfar);
decl_mesh(skydome);
decl_mesh(cubemap);
decl_mesh(chunk);
decl_mesh(2D_col);
decl_mesh(2D_tex);
decl_mesh(2D_tex_col);
decl_mesh(3D_tex);
decl_mesh(3D_tex_instanced);
decl_mesh(lines);
decl_mesh(pointcloud);

CALLBACK void uniforms_gamma(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_invert(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_resolve(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_composite(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_comp_light(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_composite_resolve(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_comp_resolve_light(shader_program* prog, render_command* cmd);

CALLBACK void run_defer(render_command* cmd, gpu_object* gpu);
CALLBACK void uniforms_defer(shader_program* prog, render_command* cmd);
CALLBACK void uniforms_defer_stencil(shader_program* prog, render_command* cmd);
