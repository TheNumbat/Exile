
#version 330 core

layout (location = 0) in uvec3 vertex;

const uint x_mask   = 0xff000000u;
const uint z_mask   = 0x00ff0000u;
const uint u_mask   = 0x0000ff00u;
const uint v_mask   = 0x000000ffu;

const uint y_mask   = 0xfff00000u;
const uint t_mask   = 0x000fff00u;
const uint ao0_mask = 0x000000c0u;
const uint ao1_mask = 0x00000030u;
const uint ao2_mask = 0x0000000cu;
const uint ao3_mask = 0x00000003u;

const uint n_mask   = 0xff000000u;
const uint l0_mask  = 0x00fc0000u;
const uint l1_mask  = 0x0003f000u;
const uint l2_mask  = 0x00000fc0u;
const uint l3_mask  = 0x0000003fu;

uniform mat4 transform;
uniform float units_per_voxel;

uniform vec4 ao_values = vec4(0.75f, 0.825f, 0.9f, 1.0f);

flat out uint f_t;
flat out vec4 f_ao;
out vec2 f_uv;

void main() {

	vec3 pos = vec3((vertex.x & x_mask) >> 24, (vertex.y & y_mask) >> 20, (vertex.x & z_mask) >> 16) / units_per_voxel;
	
	f_ao[0] = ao_values[(vertex.y & ao0_mask) >> 6];
	f_ao[1] = ao_values[(vertex.y & ao1_mask) >> 4];
	f_ao[2] = ao_values[(vertex.y & ao2_mask) >> 2];
	f_ao[3] = ao_values[(vertex.y & ao3_mask)];

	f_uv = vec2((vertex.x & u_mask) >> 8, vertex.x & v_mask) / units_per_voxel;
	f_t  = (vertex.y & t_mask) >> 8;

	gl_Position = transform * vec4(pos, 1.0);
}
