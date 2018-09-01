
#version 330 core

uniform sampler2D tex;

uniform float day_01;

in float f_sz;

out vec4 color;

void main() {

	color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
