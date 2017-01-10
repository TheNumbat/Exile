#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex;

uniform mat4 modelviewproj;

out vec2 texCoord;

void main() {
	gl_Position = modelviewproj * vec4(position, 1.0f);
	texCoord = tex;
}