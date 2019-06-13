
#version 330 core

uniform sampler2D tex;

uniform float day_01;
layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_pos;
layout (location = 2) out vec4 out_norm;
layout (location = 3) out float out_coverage;

void main() {

	out_color = vec4(1.0f, 1.0f, 1.0f, smoothstep(0.25f, 0.2f, day_01) + smoothstep(0.75f, 0.8f, day_01));
	out_pos = vec4(0.0f);
	out_norm = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	out_coverage = 0.0f;
}
