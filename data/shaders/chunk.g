
#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

const uint x_mask   = 0xff000000u;
const uint z_mask   = 0x00ff0000u;
const uint u0_mask  = 0x0000ff00u;
const uint v0_mask  = 0x000000ffu;

const uint y_mask   = 0xfff00000u;
const uint t_mask   = 0x000fff00u;
const uint ao0_mask = 0x000000c0u;
const uint ao1_mask = 0x00000030u;
const uint ao2_mask = 0x0000000cu;
const uint ao3_mask = 0x00000003u;

const uint t0_mask  = 0xff000000u;
const uint u1_mask  = 0x00ff0000u;
const uint v1_mask  = 0x0000ff00u;
const uint t1_mask  = 0x000000ffu;

const uint bd_mask 	= 0xc0000000u;
const uint ff_mask 	= 0x20000000u;
const uint bf_mask 	= 0x10000000u;

const uint l0_mask  = 0x00000000u;
const uint l1_mask  = 0x00000000u;
const uint l2_mask  = 0x00000000u;
const uint l3_mask  = 0x00000000u;

uniform float units_per_voxel;

uniform mat4 mvp;
uniform vec4 ao_values = vec4(0.75f, 0.825f, 0.9f, 1.0f);

flat in uvec4 g_vertex[];

flat out uint f_t, f_flip;
flat out vec4 f_ao;
out vec2 f_uv;
out vec3 f_n;

void main() {

	uvec4 face = g_vertex[0];

	uint bf_dim = (face.w & bd_mask) >> 30;
		 f_flip = (face.w & ff_mask) >> 29;
	uint bf0    = (face.w & bf_mask) >> 28;
	uint bf1 	= 1u - bf0;

	// Get texture info

	vec3 uv0u = vec3((face.x & u0_mask) >> 8, face.x & v0_mask, (face.z & t0_mask) >> 24);
	vec3 uv1u = vec3((face.z & u1_mask) >> 16, (face.z & v1_mask) >> 8, face.z & t1_mask);

	vec3 uv0 = vec3(uv0u.x, uv0u.y, uv0u.z) / units_per_voxel;
	vec3 uv1 = vec3(uv1u.x, uv1u.y, uv1u.z) / units_per_voxel;

	f_n = bf0 == 1u ? cross(uv0, uv1) : cross(uv1, uv0);
	f_t = (face.y & t_mask) >> 8;

	// Get vertex positions

	vec3 pos0 = vec3((face.x & x_mask) >> 24, (face.y & y_mask) >> 20, (face.x & z_mask) >> 16) / units_per_voxel;
	pos0[bf_dim] += bf0;

	vec3 pos1 = pos0 + uv0;
	vec3 pos2 = pos0 + uv1;
	vec3 pos3 = pos2 + uv0;

	// Get per-vertex data

	f_ao[0] = ao_values[(face.y & ao0_mask) >> 6];
	f_ao[1] = ao_values[(face.y & ao1_mask) >> 4];
	f_ao[2] = ao_values[(face.y & ao2_mask) >> 2];
	f_ao[3] = ao_values[(face.y & ao3_mask)];

	// Emit quad

	float uvx = length(uv0), uvy = length(uv1);

	gl_Position = mvp * vec4(pos0, 1.0);
	f_uv = vec2(0, 0);
	EmitVertex();

	gl_Position = mvp * vec4(bf0 * pos1 + bf1 * pos2, 1.0);
	f_uv = vec2(bf0 * uvx, bf1 * uvy);
	EmitVertex();
	
	gl_Position = mvp * vec4(bf1 * pos1 + bf0 * pos2, 1.0);
	f_uv = vec2(bf1 * uvx, bf0 * uvy);
	EmitVertex();
	
	gl_Position = mvp * vec4(pos3, 1.0);
	f_uv = vec2(uvx, uvy);
	EmitVertex();
	
	EndPrimitive();
}
