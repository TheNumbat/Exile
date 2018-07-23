
#version 330 core

flat in uint f_t;
flat in vec4 f_ao;
in vec2 f_uv;
in vec3 f_n;

out vec4 color;

uniform sampler2DArray tex;

void main() {

	vec3 uvt = vec3(f_uv, f_t);

	vec4 sample = texture(tex, uvt);

	float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
	float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
	float ao = mix(ao0, ao1, fract(f_uv.y));

	color = vec4(sample.rgb * ao, sample.a);
	// color = vec4(abs(f_n), 1.0f);
}
