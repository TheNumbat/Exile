
#version 330

layout (location = 0) in vec3 v_pos;

smooth out vec3 f_texcoord;

uniform mat4 transform;

void main() {
	gl_Position = transform * vec4(v_pos, 1.0);
	f_texcoord = v_pos;
}
