
#version 330 core

layout (location = 0) in vec3 v_vert;

layout (location = 1) in vec4 v_lattn;
layout (location = 2) in vec3 v_lpos;
layout (location = 3) in vec3 v_ldiff;
layout (location = 4) in vec3 v_lspec;

noperspective out vec3 f_view;
noperspective out vec2 f_uv;

flat out vec3 f_lpos, f_ldiff, f_lspec, f_lattn;
flat out float f_r;

flat out vec3 instance_col;

uniform int num_instances;
uniform mat4 vp, ivp;

vec3 scalar_to_color(float f) {
	vec3 c = vec3(4.0f * f - 2.0f, 4.0f * f + min(0.0f, 4.0f - 8.0f * f), 1.0f + 4.0f * (0.24f - f));
	return clamp(c, 0.0f, 1.0f);
}

void main() {

	vec3 vert = v_lattn.w * v_vert + v_lpos;

	vec4 clip = vp * vec4(vert, 1.0f);
	vec2 ndc = clip.xy / clip.w;

	vec4 view = ivp * vec4(ndc, 1.0f, 1.0f);
	
	gl_Position = clip;
	f_view = view.xyz;
	f_uv = ndc / 2.0f + 0.5f;
	
	instance_col = scalar_to_color(float(gl_InstanceID) / float(num_instances));

	f_lpos = v_lpos;
	f_ldiff = v_ldiff;
	f_lspec = v_lspec;
	f_lattn = v_lattn.xyz;
	f_r = v_lattn.w;
}
