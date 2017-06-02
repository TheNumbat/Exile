
#version 450 core

uniform sampler2D tex;

in vec2 uv_f;
in vec4 color_f;

out vec4 color;

void main() {
	
	color = color_f * texture(tex, uv_f);
}
