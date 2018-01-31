
#version 450 core

layout (location = 0) in uvec2 vertex;

uniform mat4 transform;

const uint l_mask = 0xff000000;
const uint z_mask = 0x00ff0000;
const uint y_mask = 0x0000ff00;
const uint x_mask = 0x000000ff;

void main() {

	vec3 pos = vec3((vertex.x & x_mask), (vertex.x & y_mask) >> 8, (vertex.x & z_mask) >> 16);
	
	gl_Position = transform * vec4(pos, 1.0);
}
