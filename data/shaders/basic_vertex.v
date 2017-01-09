#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 modelviewproj;

void main() {
	gl_Position = modelviewproj * vec4(position, 1.0f);
}