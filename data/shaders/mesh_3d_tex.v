
#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv_v;

uniform mat4 transform;

out vec2 uv_f;

void main() {
	uv_f = uv_v;
	gl_Position = transform * vec4(pos, 1.0);
}
