
#version 330 core

layout (location = 0) in uvec4 v_data;
layout (location = 1) in uvec4 qy_data;

// x z v u 
// x z v u 
// x z v u
// x z v u 
// y y 
// y y
// t ao l0
// l1 l2 l3 

uniform vec4 ao_curve;
uniform float units_per_voxel;

uniform mat4 mvp;
uniform mat4 m;

const uint x_mask   = 0xff000000u;
const uint z_mask   = 0x00ff0000u;
const uint u_mask   = 0x0000ff00u;
const uint v_mask   = 0x000000ffu;

const uvec2 y_mask  = uvec2(0xffff0000u, 0x0000ffffu);
const uvec2 y_shift = uvec2(16, 0);

const uint t_mask   = 0xffff0000u;
const uint unused   = 0x0000ff00u;
const uint ao0_mask = 0x000000c0u;
const uint ao1_mask = 0x00000030u;
const uint ao2_mask = 0x0000000cu;
const uint ao3_mask = 0x00000003u;

const uint l0_mask  = 0xff000000u;
const uint l1_mask  = 0x00ff0000u;
const uint l2_mask  = 0x0000ff00u;
const uint l3_mask  = 0x000000ffu;

flat out uint f_t;
flat out vec4 f_ao;
flat out vec4 f_l;
out vec2 f_uv;
out vec3 f_n;
out float f_ah, f_d;

struct vert {
	vec3 pos;
	vec2 uv;
};

struct quad {
	uint t;
	vec4 ao;
	vec4 l;
};

vec3 unpack_pos(int idx) {
	
	uint v = v_data[idx];
	uint y = (qy_data[idx / 2] & y_mask[idx % 2]) >> y_shift[idx % 2];

	return vec3((v & x_mask) >> 24, y, (v & z_mask) >> 16) / units_per_voxel;
}

vert v_unpack(int idx) {

	uint v = v_data[idx];
	uint y = (qy_data[idx / 2] & y_mask[idx % 2]) >> y_shift[idx % 2];

	vert o;
	
	o.pos   = vec3((v & x_mask) >> 24, y, (v & z_mask) >> 16) / units_per_voxel;
	o.uv    = vec2((v & u_mask) >> 8, v & v_mask) / units_per_voxel;

	return o;
}

quad q_unpack() {

	quad q;

	q.t = (qy_data.z & t_mask) >> 16;
	q.ao[0] = ao_curve[(qy_data.z & ao0_mask) >> 6];
	q.ao[1] = ao_curve[(qy_data.z & ao1_mask) >> 4];
	q.ao[2] = ao_curve[(qy_data.z & ao2_mask) >> 2];
	q.ao[3] = ao_curve[(qy_data.z & ao3_mask)];
	q.l[0] = (qy_data.w & l0_mask) >> 24;
	q.l[1] = (qy_data.w & l1_mask) >> 16;
	q.l[2] = (qy_data.w & l2_mask) >> 8;
	q.l[3] = (qy_data.w & l3_mask);

	return q;
}

void main() {

	// Unpack

	vert v = v_unpack(gl_VertexID);
	quad q = q_unpack();

	vec3 v1 = unpack_pos(0);
	vec3 v2 = unpack_pos(1);
	vec3 v3 = unpack_pos(2);

	vec3 m_pos = (m * vec4(v.pos, 1.0)).xyz;
	
	// Output

	gl_Position = mvp * vec4(v.pos, 1.0);

	f_n = cross(v2 - v1, v3 - v1);
	f_uv = v.uv;
	
	f_t = q.t;
	f_l = q.l;
	f_ao = q.ao;
	
	f_ah = 0.5f * (m_pos.y / length(m_pos)) + 0.5f;
	f_d = length(m_pos.xz);
}
