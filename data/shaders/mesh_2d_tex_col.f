
#version 450 core

in vec2 f_texcoord;
in vec4 f_color;

uniform sampler2D tex;

out vec4 color;

void main() {
	color = f_color * texture(tex, f_texcoord);
}
