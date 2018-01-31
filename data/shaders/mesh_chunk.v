
#version 450 core

layout (location = 0) in uvec2 vertex;

const uint n_mask = 0xff000000;
const uint z_mask = 0x00ff0000;
const uint y_mask = 0x0000ff00;
const uint x_mask = 0x000000ff;

const uint tex_uv_mask = 0xc0000000;
const uint tex_l_mask  = 0x3fff0000;
const uint face_mask   = 0x0000ff00;
const uint ao_mask     = 0x000000ff;

const vec2 uv[] = {
	vec2(0.0f, 0.0f),
	vec2(1.0f, 0.0f),
	vec2(0.0f, 1.0f),
	vec2(1.0f, 1.0f)
};

uniform mat4 transform;

flat out uint f_ao, f_tex_l, f_norm;
out vec2 f_tex_uv;

void main() {

	vec3 pos = vec3((vertex.x & x_mask), (vertex.x & y_mask) >> 8, (vertex.x & z_mask) >> 16);
	
	f_norm = (vertex.x & n_mask) >> 24;
	f_ao   = (vertex.y & ao_mask);
	
	f_tex_uv = uv[(vertex.y & tex_uv_mask) >> 30];
	f_tex_l  = (vertex.y & tex_l_mask ) >> 16;

	gl_Position = transform * vec4(pos, 1.0);
}
