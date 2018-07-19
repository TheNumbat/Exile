
#version 330 core

flat in uint f_t, f_flip;
flat in vec4 f_ao, f_l;
in vec2 f_uv;

out vec4 color;

uniform sampler2DArray tex;

void main() {

	vec3 uvt = vec3(f_flip * f_uv + (1u - f_flip) * f_uv.yx, f_t);

	vec4 sample = texture(tex, uvt);

	float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
	float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
	float ao = mix(ao0, ao1, fract(f_uv.y));

	color = vec4(sample.rgb * ao, sample.a);
}
