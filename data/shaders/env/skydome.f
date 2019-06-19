
#version 330 core

uniform sampler2D tex;

uniform float day_01;
in vec2 f_texcoord;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_pos;
layout (location = 2) out vec4 out_norm;
layout (location = 3) out vec4 out_light;

void main() {

	vec2 sky_uv = vec2(day_01, f_texcoord.y);

	out_color = texture(tex, sky_uv);

	vec4 clear = vec4(vec3(0.0f), 1.0f);
	out_pos = clear;
	out_norm = clear;
	out_light = clear;
}
