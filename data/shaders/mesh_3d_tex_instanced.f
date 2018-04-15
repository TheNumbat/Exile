
#version 330 core

uniform sampler2D tex;

in vec2 f_texcoord;
out vec4 color;

void main() {
	color = texture(tex, f_texcoord);
}
