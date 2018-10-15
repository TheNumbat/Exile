
#version 330 core

uniform sampler2D tex;

uniform float day_01;
in vec2 f_tc;

out vec4 color;

void main() {

	color = texture(tex, f_tc);
}
