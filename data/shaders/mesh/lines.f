
#version 400 core

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_norm;
layout (location = 2) out vec4 out_light;

in vec4 f_color;

void main() {
	gl_SampleMask[0] = 0xffffffff;
	out_color = f_color;
	out_norm = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	out_light = vec4(1.0f);
}
