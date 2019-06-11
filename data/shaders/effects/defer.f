
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform sampler2D col_tex;
uniform sampler2D pos_tex;
uniform sampler2D norm_tex;

void main() {

	vec3 c = texture(col_tex, f_uv).rgb;
	vec3 p = texture(pos_tex, f_uv).rgb;
	vec3 n = texture(norm_tex, f_uv).rgb;

	color = vec4(c, 1.0f);
}

