
#version 450 core

layout (location = 0) in uvec2 vertex;

const uint y_mask = 0xfff00000;
const uint n_mask = 0x000f0000;
const uint z_mask = 0x0000ff00;
const uint x_mask = 0x000000ff;

const uint t_mask  = 0x00000fff;
const uint ao_mask = 0x0000f000;
const uint u_mask  = 0x00ff0000;
const uint v_mask  = 0xff000000;

uniform mat4 transform;

flat out uint f_t, f_norm;
out float f_ao;
out vec2 f_uv;

float ao_values[4] = {
	0.4f,
	0.6f,
	0.8f,
	1.0f
};

void main() {

	vec3 pos = vec3((vertex.x & x_mask), (vertex.x & y_mask) >> 20, (vertex.x & z_mask) >> 8);
	
	f_ao = ao_values[(vertex.y & ao_mask) >> 12];
	f_norm = (vertex.x & n_mask) >> 16;
	
	f_uv.x = (vertex.y & u_mask) >> 16;
	f_uv.y = (vertex.y & v_mask) >> 24;
	f_t    = (vertex.y & t_mask);

	gl_Position = transform * vec4(pos, 1.0);
}
