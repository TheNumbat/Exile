
#version 330 core

flat in uint f_t, f_norm;
in float f_ao;
in vec2 f_uv;

out vec4 color;

uniform sampler2DArray tex;

void main() {

	vec4 tex = texture(tex, vec3(f_uv, f_t));

	color = vec4(f_ao, f_ao, f_ao, tex.a);
	// color = vec4(tex.rgb * f_ao, tex.a);
}
