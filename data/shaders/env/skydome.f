
#version 330 core

uniform sampler2D tex;

uniform float day_01;
in vec2 f_texcoord;

out vec4 color;

void main() {

	vec2 sky_uv = vec2(day_01, f_texcoord.y);

	color = texture(tex, sky_uv);
}
