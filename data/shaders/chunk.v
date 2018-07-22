
#version 330 core

layout (location = 0) in uvec4 vertex;

flat out uvec4 g_vertex;

void main() {
	g_vertex = vertex;
}
