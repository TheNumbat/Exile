
#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 mvp;

flat out uint f_t;
flat out vec4 f_ao;
out vec2 f_uv;
out vec3 f_n;

struct vert {
	vec3 pos;
	vec2 uv;
	vec4 ao;
	uint t;
};

flat in vert g_v0[], g_v1[], g_v2[], g_v3[];

void send(vert v) {

	gl_Position = mvp * vec4(v.pos, 1.0);
	f_uv = v.uv;
	f_ao = v.ao;
	f_t = v.t;
	EmitVertex();
}

void main() {

	vert v0 = g_v0[0];
	vert v1 = g_v1[0];
	vert v2 = g_v2[0];
	vert v3 = g_v3[0];

	f_n = cross(v1.pos - v0.pos, v2.pos - v0.pos);
	
	send(v0);
	send(v1);
	send(v2);
	send(v3);
	EndPrimitive();
}
