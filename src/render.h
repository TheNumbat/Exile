
#pragma once

typedef u32 shader_program_id;
typedef u32 texture_id;
typedef u32 context_id;

enum class render_command_type : u8 {
	none,
	mesh_2d,
	mesh_3d,
};

struct mesh_2d {
	vector<v2>		verticies;	// x y 
	vector<v3>		texCoords;	// u v
	vector<colorf>	colors;		// r g b a (clamp f)
	vector<uv3> 	elements;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static mesh_2d make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	
	void clear();

	f32 push_text_line(asset* font, string text_utf8, v2 pos, f32 point = 0.0f, color c = V4b(255, 255, 255, 255)); 
	void push_rect(r2 rect, color c);
	void push_cutrect(r2 r, f32 round, color c);
};

struct mesh_3d {
	vector<v3>  verticies;	// x y z
	vector<v2>  texCoords; 	// u v (layer)
	// TODO(max): indices
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

	static mesh_3d make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	
	void clear();
};

struct render_command {
	render_command_type cmd 	= render_command_type::none;
	shader_program_id 	shader 	= 0;
	texture_id 			texture = 0;
	context_id 			context = 0;
	m4 model;
	u32 elements = 0;
	u32 sort_key = 0;
	union {
		void* data = null;
		mesh_3d*	m3d;
		mesh_2d* 	m2d;
	};

///////////////////////////////////////////////////////////////////////////////

	static render_command make(render_command_type type, void* data, u32 key = 0);
};

struct render_camera {

};

struct render_command_list {
	vector<render_command> commands;
	render_camera cam;
	allocator* alloc = null;
	m4 view;
	m4 proj;

///////////////////////////////////////////////////////////////////////////////

	static render_command_list make(allocator* alloc = null, u32 cmds = 8);
	void destroy();
	void add_command(render_command rc);
	void sort();
};

v2 size_text(asset* font, string text_utf8, f32 point);

bool operator<(render_command first, render_command second);

