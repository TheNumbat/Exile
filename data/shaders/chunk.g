
#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

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

const uint d0_mask  = 0xc0000000u;
const uint d1_mask  = 0x30000000u;
const uint d2_mask  = 0x0c000000u;
const uint b_mask   = 0x03000000u;
const uint l0_mask  = 0x00fc0000u;
const uint l1_mask  = 0x0003f000u;
const uint l2_mask  = 0x00000fc0u;
const uint l3_mask  = 0x0000003fu;

uniform float units_per_voxel;

uniform mat4 transform;
uniform vec4 ao_values = vec4(0.75f, 0.825f, 0.9f, 1.0f);

flat in uvec3 g_vertex[];

flat out uint f_t, f_flip;
flat out vec4 f_ao, f_l;
out vec2 f_uv;
out vec3 f_n;

void main() {

	uvec3 face = g_vertex[0];

	// All the dimension/backface info could be reconstructed from i 0..6 (direction) to save 4 bits, giving us an extra lighting bit per vertex.
	// However, that would also mean we have to do some modular division here, which should probably be kept out of the shaders
	// as much as possible. Might just add it, though, if we want more lighting fidelity.

	// Get dimension info

	uint d0 = (face.z & d0_mask) >> 30;
	uint d1 = (face.z & d1_mask) >> 28;
	uint d2 = (face.z & d2_mask) >> 26;
	
	// Get branch-less backface factor

	uint b0 = (face.z & b_mask) >> 24;
	uint b1 = 1u - b0;

	// Flip texture on x-facing quads

	f_flip = d1 & 1u;

	// Get texture info

	vec2 uv = (vec2((face.x & u_mask) >> 8, face.x & v_mask) + vec2(1, 1)) / units_per_voxel;
	f_t  = (face.y & t_mask) >> 8;

	// Get vertex positions

	vec3 pos0 = vec3((face.x & x_mask) >> 24, (face.y & y_mask) >> 20, (face.x & z_mask) >> 16) / units_per_voxel;
	pos0[d2] += b0;

	vec3 pos1 = pos0; pos1[d0] += uv.x;
	vec3 pos2 = pos0; pos2[d1] += uv.y;
	vec3 pos3 = pos2; pos3[d0] += uv.x;

	f_n = cross(pos1 - pos0, pos2 - pos0);

	// Get per-vertex data

	f_ao[0] = ao_values[(face.y & ao0_mask) >> 6];
	f_ao[1] = ao_values[(face.y & ao1_mask) >> 4];
	f_ao[2] = ao_values[(face.y & ao2_mask) >> 2];
	f_ao[3] = ao_values[(face.y & ao3_mask)];

	f_l[0] = float((face.z & l0_mask) >> 18) / 64.0;
	f_l[1] = float((face.z & l1_mask) >> 12) / 64.0;
	f_l[2] = float((face.z & l2_mask) >> 6) / 64.0;
	f_l[3] = float(face.z & l3_mask) / 64.0;

	// Emit quad

	gl_Position = transform * vec4(pos0, 1.0);
	f_uv = vec2(0, 0);
	EmitVertex();

	gl_Position = transform * vec4(b0 * pos1 + b1 * pos2, 1.0);
	f_uv = vec2(b0 * uv.x, b1 * uv.y);
	EmitVertex();
	
	gl_Position = transform * vec4(b1 * pos1 + b0 * pos2, 1.0);
	f_uv = vec2(b1 * uv.x, b0 * uv.y);
	EmitVertex();
	
	gl_Position = transform * vec4(pos3, 1.0);
	f_uv = uv;
	EmitVertex();
	
	EndPrimitive();
}
