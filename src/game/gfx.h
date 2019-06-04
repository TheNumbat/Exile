
#pragma once

struct chunk_quad {

	u8  z_1, x_1, z_0, x_0;
	u8  z_3, x_3, z_2, x_2;
	u32 uy01;
	u32 vy23;

	u16 aol, t;
	u8 l3, l2, l1, l0;
	u8 s3, s2, s1, s0;
};
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

struct mesh_cubemap {
	v3 vertices[36] = {
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
	bool dirty = false;

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

	void push(v3 p1, v3 p2, colorf c1, colorf c2);
	void push(v3 p1, v3 p2, colorf c);
};
CALLBACK void destroy_lines(mesh_lines* m);

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
CALLBACK void destroy_2d_col(mesh_2d_col* m);

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

// NOTE(max): sort of a client implementation using the engine OGL renderer 
struct exile_renderer {

	static exile_renderer make();
	void destroy();

	draw_cmd_id cmd_2d_col           = -1;
	draw_cmd_id cmd_2d_tex           = -1;
	draw_cmd_id cmd_2d_tex_col       = -1;
	draw_cmd_id cmd_3d_tex           = -1;
	draw_cmd_id cmd_3d_tex_instanced = -1;
	draw_cmd_id cmd_lines            = -1;
	draw_cmd_id cmd_pointcloud       = -1;
	draw_cmd_id cmd_cubemap          = -1;
	draw_cmd_id cmd_chunk            = -1;
	draw_cmd_id cmd_skydome          = -1;
	draw_cmd_id cmd_skyfar           = -1;

	texture_id 	   world_buf_tex = -1;
	framebuffer_id world_buffer  = -1;

	render_command hud_2D_cmd(mesh_2d_col* mesh);

	render_command world_lines_cmd(mesh_lines* mesh, m4 view, m4 proj);
	render_command world_stars_cmd(gpu_object_id gpu_id, world_time* time, m4 view, m4 proj);
	render_command world_skydome_cmd(gpu_object_id gpu_id, world_time* time, texture_id sky, m4 view, m4 proj);
	render_command world_chunk_cmd(world* w, chunk* c, texture_id blocks, texture_id sky, m4 model, m4 view, m4 proj);

	void generate_mesh_commands();
	void generate_swapchain();

	void render_to_screen();
};

#define decl_mesh(name) \
	CALLBACK void uniforms_mesh_##name(shader_program* prog, render_command* cmd); 	\
	CALLBACK void setup_mesh_##name(gpu_object* obj); 								\
	CALLBACK void update_mesh_##name(gpu_object* obj, void* data, bool force);		\
	CALLBACK void run_mesh_##name(render_command* cmd, gpu_object* gpu);			\
	CALLBACK bool compat_mesh_##name(ogl_info* info);

decl_mesh(skyfar);
decl_mesh(skydome);
decl_mesh(cubemap);
decl_mesh(chunk);
decl_mesh(2d_col);
decl_mesh(2d_tex);
decl_mesh(2d_tex_col);
decl_mesh(3d_tex);
decl_mesh(3d_tex_instanced);
decl_mesh(lines);
decl_mesh(pointcloud);
