
#pragma once

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

///////////////////////////////////////////////////////////////////////////////

	static mesh_2d_col make(u32 verts = 32, allocator* alloc = null);
	void destroy();
	void clear();

	void push_rect(r2 rect, color c);
	void push_cutrect(r2 r, f32 round, color c);
};

struct mesh_2d_tex {
	vector<v2>		vertices;	// x y 
	vector<v2>		texCoords;	// u v
	vector<uv3> 	elements;
	allocator* alloc = null;

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

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
	u32 start_tri = 0, num_tris = 0;

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

