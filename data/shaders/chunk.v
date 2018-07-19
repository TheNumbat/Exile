
#version 330 core

layout (location = 0) in uvec3 vertex;

flat out uvec3 g_vertex;

void main() {
	g_vertex = vertex;
}
