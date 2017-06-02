
#pragma once

enum render_command_type {
	render_none,
	render_mesh,
	render_mesh_2d,
};

struct mesh_2d {
	vector<v2>		verticies;	// x y
	vector<v3>		texCoords;	// u v layer
	vector<colorf>	colors;		// r g b a
};

struct mesh {
	vector<v3>  verticies;	// x y z
	vector<v3>  texCoords; 	// u v layer
	vector<uv3>	indicies;	// x y z
	m4 			model;
};

struct render_command {
	render_command_type cmd = render_none;
	union {
		mesh* 		m 	= NULL;
		mesh_2d* 	m2d;
	};
};

struct render_camera {

};

struct render_command_list {
	vector<render_command> commands;
	render_camera cam;
	m4 proj;
};

