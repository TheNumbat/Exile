
#version 330 core

layout (location = 0) in vec3 pos;

out float f_sz;

uniform mat4 transform;

void main() {
	
	vec4 pos = transform * vec4(pos, 1.0);
	
	gl_Position = pos.xyww;
}
