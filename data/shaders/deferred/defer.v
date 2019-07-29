
#version 330 core

layout (location = 0) in vec3 v_vert;

layout (location = 1) in vec3 v_lpos;
layout (location = 2) in vec3 v_lcol;

noperspective out vec3 f_view;
noperspective out vec2 f_uv;

flat out vec3 f_lpos, f_lcol;

uniform mat4 vp, ivp;

void main() {

	float r = max(v_lcol.x,max(v_lcol.y,v_lcol.z));	

	vec3 vert = 2.0f * r * v_vert + v_lpos;

	vec4 clip = vp * vec4(vert, 1.0f);
	vec2 ndc = clip.xy / clip.w;

	vec4 view = ivp * vec4(ndc, 1.0f, 1.0f);
	
	gl_Position = clip;
	f_view = view.xyz;
	f_uv = ndc / 2.0f + 0.5f;
	f_lpos = v_lpos;
	f_lcol = v_lcol;
}
