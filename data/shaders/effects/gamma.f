
#version 330 core

in vec2 f_uv;
out vec4 color;

uniform sampler2D tex;
uniform float gamma;

void main() {

	color = vec4(pow(texture(tex, f_uv).rgb, vec3(1.0f / gamma)), 1.0f);
}

