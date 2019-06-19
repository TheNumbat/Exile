
#version 330 core

layout (location = 0) in uvec4 v_data;
layout (location = 1) in uvec3 q_data;

uniform vec4 ao_curve;
uniform float units_per_voxel;

uniform mat4 mvp;
uniform mat4 m;

const vec4 comp1 = vec4(0, 1, 0, 1);
const vec4 comp2 = vec4(0, 0, 1, 1);

const uvec2 x_mask  = uvec2(0xff000000u, 0x0000ff00u);
const uvec2 x_shift = uvec2(24, 8);

const uvec2 y_mask  = uvec2(0xfff00000u, 0x000fff00u);
const uvec2 y_shift = uvec2(20, 8);

const uint u_mask   = 0x000000ffu;
const uint v_mask   = 0x000000ffu;

const uvec2 z_mask  = uvec2(0x00ff0000u, 0x000000ffu);
const uvec2 z_shift = uvec2(16, 0);

const uint t_mask   = 0xffff0000u;
const uint qs_mask  = 0x0000f000u;
const uint ql_mask  = 0x00000f00u;
const uint ao0_mask = 0x000000c0u;
const uint ao1_mask = 0x00000030u;
const uint ao2_mask = 0x0000000cu;
const uint ao3_mask = 0x00000003u;

const uint l0_mask  = 0x0f000000u;
const uint l1_mask  = 0x000f0000u;
const uint l2_mask  = 0x00000f00u;
const uint l3_mask  = 0x0000000fu;

const uint s0_mask  = 0xf0000000u;
const uint s1_mask  = 0x00f00000u;
const uint s2_mask  = 0x0000f000u;
const uint s3_mask  = 0x000000f0u;

flat out uint f_t, f_ql, f_qs;
flat out vec4 f_ao;
flat out vec4 f_l, f_s;
out vec2 f_uv;
out vec3 f_n;
out vec3 f_pos;

struct vert {
	vec3 pos;
	vec2 uv;
};

struct quad {
	uint t;
	uint ql;
	uint qs;
	vec4 ao;
	vec4 l;
	vec4 s;
	vec2 uv;
};

vec3 unpack(int idx0, int idx1) {

	uint x = (v_data[idx0] & x_mask[idx1]) >> x_shift[idx1];
	uint y = (v_data[idx0 + 2] & y_mask[idx1]) >> y_shift[idx1];
	uint z = (v_data[idx0] & z_mask[idx1]) >> z_shift[idx1];

	return vec3(x, y, z) / units_per_voxel;
}

quad q_unpack() {

	quad q;

	q.t = (q_data.x & t_mask) >> 16;
	q.ao[0] = ao_curve[(q_data.x & ao0_mask) >> 6];
	q.ao[1] = ao_curve[(q_data.x & ao1_mask) >> 4];
	q.ao[2] = ao_curve[(q_data.x & ao2_mask) >> 2];
	q.ao[3] = ao_curve[(q_data.x & ao3_mask)];
	
	q.ql = (q_data.x & ql_mask) >> 8;
	q.qs = (q_data.x & qs_mask) >> 12;
	
	q.l[0] = (q_data.y & l0_mask) >> 24;
	q.l[1] = (q_data.y & l1_mask) >> 16;
	q.l[2] = (q_data.y & l2_mask) >> 8;
	q.l[3] = (q_data.y & l3_mask);
	q.l /= 16.0f;

	q.s[0] = (q_data.y & s0_mask) >> 28;
	q.s[1] = (q_data.y & s1_mask) >> 20;
	q.s[2] = (q_data.y & s2_mask) >> 12;
	q.s[3] = (q_data.y & s3_mask) >> 4;
	q.s /= 16.0f;
	
	q.uv[0] = (v_data.z & u_mask) / units_per_voxel;
	q.uv[1] = (v_data.w & v_mask) / units_per_voxel;

	return q;
}

void main() {

	// Unpack

	int idx0 = gl_VertexID >> 1;
	int idx1 = gl_VertexID & 1;

	quad q = q_unpack();

	vec3 v0 = unpack(idx0, idx1);
	vec3 v1 = unpack(0, 0);
	vec3 v2 = unpack(0, 1);
	vec3 v3 = unpack(1, 0);

	vec3 m_pos = (m * vec4(v0, 1.0)).xyz;
	f_pos = m_pos;
	
	// Output

	gl_Position = mvp * vec4(v0, 1.0);

	f_n = cross(v2 - v1, v3 - v1);
	f_uv = q.uv * vec2(comp1[gl_VertexID], comp2[gl_VertexID]);
	
	f_t = q.t;
	f_l = q.l;
	f_s = q.s;
	f_ql = q.ql;
	f_qs = q.qs;
	f_ao = q.ao;
}

