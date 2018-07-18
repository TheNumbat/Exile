
#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 g_texcoord[];
out vec2 f_texcoord[];

void main() {

	for(int i = 0; i < gl_in.length(); i++) {

		gl_Position = gl_in[i].gl_Position;
		f_texcoord[i] = g_texcoord[i];

		EmitVertex();
	}

	EndPrimitive();
}
