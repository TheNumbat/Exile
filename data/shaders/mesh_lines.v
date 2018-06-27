
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 v_color;

uniform mat4 transform;

out vec4 f_color;

void main() {
	gl_Position = transform * vec4(pos, 1.0);
	f_color = v_color;
}
