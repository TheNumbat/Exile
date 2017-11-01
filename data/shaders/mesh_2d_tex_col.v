
#version 450 core

layout (location = 0) in vec2 v_pos;
layout (location = 1) in vec2 v_texcoord;
layout (location = 2) in vec4 v_color;

out vec2 f_texcoord;
out vec4 f_color;

uniform mat4 transform;

void main() {
	gl_Position = transform * vec4(v_pos, 0.0, 1.0);
	f_color = v_color;
	f_texcoord = v_texcoord;
}
