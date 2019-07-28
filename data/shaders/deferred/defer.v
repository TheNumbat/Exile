
#version 330 core

layout (location = 0) in vec3 v_vert;

uniform vec3 lpos;
uniform vec3 lcol;

out vec3 f_view;
out vec2 f_uv;

uniform mat4 vp, ivp;

void main() {

	float r = max(lcol.x,max(lcol.y,lcol.z));	

	vec3 vert = 2.0f * r * v_vert + lpos;

	vec4 ndc = vp * vec4(vert, 1.0f);

	gl_Position = ndc;
	
	f_view = (ivp * vec4(ndc.xy, 1.0f, 1.0f)).xyz;
	f_uv = ndc.xy / 2.0f + 0.5f;
}
