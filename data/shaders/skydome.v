
#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 v_texcoord;

uniform mat4 transform;

out vec2 f_texcoord;

void main() {
	vec4 pos = transform * vec4(pos, 1.0);
	gl_Position = pos.xyww;

	f_texcoord = v_texcoord;
}
