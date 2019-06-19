
#version 330 core

flat in uint f_t, f_ql, f_qs;
flat in vec4 f_ao;
flat in vec4 f_l, f_s;
in vec2 f_uv;
in vec3 f_n;
in vec3 f_pos;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_pos;
layout (location = 2) out vec4 out_norm;
layout (location = 3) out vec4 out_light;

uniform sampler2DArray blocks_tex;

uniform bool smooth_light;

void main() {

	vec3 uvt = vec3(f_uv, f_t);
	
	out_color = texture(blocks_tex, uvt);
	out_pos = vec4(f_pos, 1.0f);
	
	float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
	float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
	out_light.w = 1.0f;

	out_norm = vec4(normalize(f_n), 1.0f);
	out_light.z = (2.0f * step(0.0f, f_n.z) - 1.0f) * mix(ao0, ao1, fract(f_uv.y));

	if(smooth_light) {

		float t0 = mix(f_l.x, f_l.y, fract(f_uv.x));
		float t1 = mix(f_l.z, f_l.w, fract(f_uv.x));
		float t  = mix(t0, t1, fract(f_uv.y));

		float s0 = mix(f_s.x, f_s.y, fract(f_uv.x));
		float s1 = mix(f_s.z, f_s.w, fract(f_uv.x));
		float s  = mix(s0, s1, fract(f_uv.y));

		out_light.x = t;
		out_light.y = s;

	} else {

		float t = float(f_ql) / 15.0f;
		float s = float(f_qs) / 15.0f;
		out_light.x = t;
		out_light.y = s;
	}
}
