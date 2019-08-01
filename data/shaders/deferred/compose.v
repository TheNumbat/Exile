
#version 330 core

layout (location = 0) in vec2 v_pos;
layout (location = 1) in vec2 v_uv;

out vec2 f_uv;
out vec3 f_view;

uniform mat4 ivp;

void main() {

	gl_Position = vec4(v_pos, 1.0f, 1.0f);
	
	f_uv = v_uv;
	f_view = (ivp * vec4(v_pos, 1.0f, 1.0f)).xyz;
}
