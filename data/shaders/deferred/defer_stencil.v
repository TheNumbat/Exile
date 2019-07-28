
#version 330 core

layout (location = 0) in vec3 v_vert;

uniform vec3 lpos;
uniform vec3 lcol;
uniform mat4 vp;

void main() {

	float r = max(lcol.x,max(lcol.y,lcol.z));	

	vec3 vert = 2.0f * r * v_vert + lpos;
	gl_Position = vp * vec4(vert, 1.0f);
}
