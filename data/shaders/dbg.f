#version 450 core

uniform sampler2D tex;

in vec2 uv;
out vec4 color;

void main() {
	
	color = texture(tex, uv);
}
