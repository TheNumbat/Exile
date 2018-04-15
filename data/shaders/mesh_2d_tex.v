
#version 330 core

layout (location = 0) in vec2 v_pos;
layout (location = 1) in vec2 v_texcoord;

out vec2 f_texcoord;

uniform mat4 transform;

void main() {
	gl_Position = transform * vec4(v_pos, 0.0, 1.0);
	f_texcoord = v_texcoord;
}
