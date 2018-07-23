
#version 330

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

flat in uvec2 g_v0[], g_v1[], g_v2[], g_v3[];

uniform mat4 mvp;
uniform float units_per_voxel;

uniform vec4 ao_values = vec4(0.75f, 0.825f, 0.9f, 1.0f);

flat out uint f_t;
flat out vec4 f_ao;
out vec2 f_uv;
out vec3 f_n;

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

struct vert {
	vec3 pos;
	vec2 uv;
	vec4 ao;
	uint t;
};

vert unpack(uvec2 i) {

	vert o;
	
	o.pos   = vec3((i.x & x_mask) >> 24, (i.y & y_mask) >> 20, (i.x & z_mask) >> 16) / units_per_voxel;
	o.uv    = vec2((i.x & u_mask) >> 8, i.x & v_mask) / units_per_voxel;
	o.t     = (i.y & t_mask) >> 8;
	o.ao[0] = ao_values[(i.y & ao0_mask) >> 6];
	o.ao[1] = ao_values[(i.y & ao1_mask) >> 4];
	o.ao[2] = ao_values[(i.y & ao2_mask) >> 2];
	o.ao[3] = ao_values[(i.y & ao3_mask)];

	return o;
}

void send(vert v) {

	gl_Position = mvp * vec4(v.pos, 1.0);
	f_uv = v.uv;
	f_ao = v.ao;
	f_t = v.t;
	EmitVertex();
}

void main() {

	vert v0 = unpack(g_v0[0]);
	vert v1 = unpack(g_v1[0]);
	vert v2 = unpack(g_v2[0]);
	vert v3 = unpack(g_v3[0]);

	f_n = cross(v1.pos - v0.pos, v2.pos - v0.pos);
	
	send(v0);
	send(v1);
	send(v2);
	send(v3);
	EndPrimitive();
}
