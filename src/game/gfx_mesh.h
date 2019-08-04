
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

#pragma pack(push, 1)
struct light {
	v3 position;
	v3 direction; 
	v3 diffuse;
	v3 specular;
	v2 cutoff;
	v4 atten;    // constant linear quadratic radius
};
#pragma pack(pop)

struct sun_light {
	v3 direction;
	v3 diffuse;
	v3 specular;
};

struct mesh_light_list {
	vector<light> lights;
	
	inline static const f32 R_MAX = 1000.0f;
	inline static const f32 t = (1.0f + sqrtf(5.0f)) / 2.0f;
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

	void push_point(v3 pos, v3 diff, v3 spec, v3 attn);
	void push_spot(v3 pos, v3 dir, v3 diff, v3 spec, v2 cut, v3 attn);
	static f32 calc_r(v3 diff, v3 spec, v3 attn, f32 cut);
	
	void init(allocator* alloc = null);
	void destroy();
	void clear();
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
