
#version 330 core

flat in uint f_t;
flat in vec4 f_ao;
in vec2 f_uv;

out vec4 color;

uniform sampler2DArray tex;

void main() {

	vec4 tex = texture(tex, vec3(f_uv, f_t));

	float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
	float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
	float ao = mix(ao0, ao1, fract(f_uv.y));

	color = vec4(vec3(1.0f) * ao, tex.a);
}
