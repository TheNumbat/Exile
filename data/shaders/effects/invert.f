
#version 330 core

in vec2 f_uv;
out vec4 color;

uniform sampler2D tex;

void main() {

	color = vec4(vec3(1.0f) - texture(tex, f_uv).rgb, 1.0f);
}

