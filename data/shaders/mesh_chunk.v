
#version 330 core

layout (location = 0) in uvec2 vertex;

const uint y_mask = 0xfff00000u;
const uint z_mask = 0x0000ff00u;
const uint x_mask = 0x000000ffu;

const uint t_mask = 0x00000fffu;
const uint u_mask = 0x00ff0000u;
const uint v_mask = 0xff000000u;

const uint ao0_mask = 0x000c0000u;
const uint ao1_mask = 0x00030000u;
const uint ao2_mask = 0x0000c000u;
const uint ao3_mask = 0x00003000u;

uniform mat4 transform;

uniform vec4 ao_values = vec4(0.65f, 0.8f, 0.9f, 1.0f);

flat out uint f_t;
flat out vec4 f_ao; 
out vec2 f_uv;

void main() {

	vec3 pos = vec3((vertex.x & x_mask), (vertex.x & y_mask) >> 20, (vertex.x & z_mask) >> 8);
	
	f_ao[0] = ao_values[(vertex.x & ao0_mask) >> 18];
	f_ao[1] = ao_values[(vertex.x & ao1_mask) >> 16];
	f_ao[2] = ao_values[(vertex.y & ao2_mask) >> 14];
	f_ao[3] = ao_values[(vertex.y & ao3_mask) >> 12];

	f_uv.x = (vertex.y & u_mask) >> 16;
	f_uv.y = (vertex.y & v_mask) >> 24;
	f_t    = (vertex.y & t_mask);

	gl_Position = transform * vec4(pos, 1.0);
}
