
#version 330 core

uniform sampler2D sky;
uniform sampler2D night_sky;

uniform float day_01;
in vec2 f_texcoord;

out vec4 color;

void main() {

	vec2 sky_uv = vec2(day_01, f_texcoord.y);

	color = texture(sky, sky_uv);

	// float remap = acos(2.0f * (f_texcoord.y - 0.5f)) / 3.14159f;
	// color = texture(night_sky, vec2(f_texcoord.x, remap));
}
