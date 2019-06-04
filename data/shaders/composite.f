
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

void main() {

	color = texture(tex0, f_uv);
}

