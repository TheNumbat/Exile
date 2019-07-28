
#version 330 core

layout (location = 0) in vec3 v_vert;

uniform vec3 lpos;
uniform vec3 lcol;

noperspective out vec3 f_view;
noperspective out vec2 f_uv;

uniform mat4 vp, ivp;

void main() {

	float r = max(lcol.x,max(lcol.y,lcol.z));	

	vec3 vert = 2.0f * r * v_vert + lpos;

	vec4 clip = vp * vec4(vert, 1.0f);

	gl_Position = clip;
	
	vec2 ndc = clip.xy / clip.w;

	vec4 view = ivp * vec4(ndc, 1.0f, 1.0f);
	f_view = view.xyz;

	f_uv = ndc / 2.0f + 0.5f;
}
