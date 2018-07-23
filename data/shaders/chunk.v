
#version 330 core

layout (location = 0) in uvec4 v_v0;

uniform float units_per_voxel;

uniform vec4 ao_values = vec4(0.75f, 0.825f, 0.9f, 1.0f);

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

flat out vert g_v0, g_v1;

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

void main() {
	g_v0 = unpack(v_v0.xy);
	g_v1 = unpack(v_v0.zw);
}
