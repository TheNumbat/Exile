
#version 330 core

uniform sampler2D tex;

uniform float day_01;
layout (location = 0) out vec4 out_color;

void main() {

	out_color = vec4(1.0f, 1.0f, 1.0f, smoothstep(0.25f, 0.2f, day_01) + smoothstep(0.75f, 0.8f, day_01));
}
