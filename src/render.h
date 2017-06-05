
#pragma once

typedef u32 shader_program_id;
typedef u32 texture_id;
typedef u32 context_id;

enum render_command_type {
	render_none,
	render_mesh_2d,
	render_mesh_3d,
};

struct mesh_2d {
	vector<v2>		verticies;	// x y 
	vector<v2>		texCoords;	// u v
	vector<colorf>	colors;		// r g b a (clamp f)
	allocator* alloc = NULL;
};

struct mesh_3d {
	vector<v3>  verticies;	// x y z
	vector<v2>  texCoords; 	// u v (layer)
	// TODO(max): indices
	allocator* alloc = NULL;
};

struct render_command {
	render_command_type cmd 	= render_none;
	shader_program_id 	shader 	= 0;
	texture_id 			texture = 0;
	context_id 			context = 0;
	m4 model;
	union {
		void* data = NULL;
		mesh_3d*	m3d;
		mesh_2d* 	m2d;
	};
};

struct render_camera {

};

struct render_command_list {
	vector<render_command> commands;
	render_camera cam;
	allocator* alloc = NULL;
	m4 view, proj;
};

mesh_2d make_mesh_2d(allocator* alloc, u32 verts = 32);
void destroy_mesh_2d(mesh_2d* m);

void mesh_push_text(mesh_2d* m, string text_utf8);

mesh_3d make_mesh_3d(allocator* alloc, u32 verts = 32, u32 inds = 32);
void destroy_mesh_3d(mesh_3d* m);

render_command make_render_command(render_command_type type, void* data);

render_command_list make_command_list(allocator* alloc, u32 cmds = 64);
void destroy_command_list(render_command_list* rcl);
void render_add_command(render_command_list* rcl, render_command rc);
