
#version 450 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec3 uv_v;
layout (location = 2) in vec4 color_v;

uniform mat4 transform;

out vec3 uv_f;
out vec4 color_f;

void main() {

	uv_f = uv_v;
	color_f = color_v;
	gl_Position = transform * vec4(pos, 0.0, 1.0);
}
