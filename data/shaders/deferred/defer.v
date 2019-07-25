
#version 330 core

layout (location = 0) in vec3 v_vert;

layout (location = 1) in vec3 v_lpos;
layout (location = 2) in vec3 v_lcol;

out vec3 f_view;
flat out vec3 f_lpos;
flat out vec3 f_lcol;
flat out int instance_id;

uniform mat4 vp, ivp;

void main() {

	float r = max(v_lcol.x,max(v_lcol.y,v_lcol.z));	

	vec3 vert = v_vert + v_lpos;

	vec4 ndc = vp * vec4(vert, 1.0f);

	gl_Position = ndc;

	f_view = (ivp * vec4(ndc.xy,1.0f,1.0f)).xyz;

	f_lpos = v_lpos;
	f_lcol = v_lcol;
	instance_id = gl_InstanceID;
}
