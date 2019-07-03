
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

uniform float day_01;
uniform float ambient;
uniform bool smooth_light;
uniform bool block_light;
uniform bool ambient_occlusion;

uniform int debug_show;

vec3 calculate_light_base(vec3 light) {

	vec3 result = vec3(0.0f);

	if(block_light) {
		float day_factor = 1.0f - (smoothstep(0.32f, 0.17f, day_01) + smoothstep(0.75f, 0.9f, day_01));
		float l = max(light.x, light.y * day_factor);
		result += vec3(pow(l,3));
	} 

	if(ambient_occlusion) {
		result *= abs(light.z);
	}

	return ambient + result;
}

void main() {

	vec3 uvt = vec3(f_uv, f_t);
	
	out_color = texture(blocks_tex, uvt);
	out_pos = vec4(f_pos, 1.0f);
	
	vec2 norm_xy = normalize(f_n).xy;
	float norm_sign = 2.0f * step(0.0f, f_n.z) - 1.0f;

	float shiny = 1.0f / 32.0f; // TODO(max): materials
	out_norm = vec4(norm_xy, norm_sign * shiny, 1.0f);
	
	float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
	float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
	float ao = mix(ao0, ao1, fract(f_uv.y));
	float t, s;

	if(smooth_light) {

		float t0 = mix(f_l.x, f_l.y, fract(f_uv.x));
		float t1 = mix(f_l.z, f_l.w, fract(f_uv.x));
		t = mix(t0, t1, fract(f_uv.y));

		float s0 = mix(f_s.x, f_s.y, fract(f_uv.x));
		float s1 = mix(f_s.z, f_s.w, fract(f_uv.x));
		s = mix(s0, s1, fract(f_uv.y));

	} else {

		t = float(f_ql) / 15.0f;
		s = float(f_qs) / 15.0f;
	}

	vec3 result = calculate_light_base(vec3(t, s, ao));

	if(debug_show < 5) {
		out_light = vec4(result, 1.0f);
	} else if(debug_show == 5) {
		out_light = vec4(vec3(pow(t,3)), 1.0f);
	} else if(debug_show == 6) {
		out_light = vec4(vec3(pow(s,3)), 1.0f);
	} else if(debug_show == 7) {
		out_light = vec4(vec3(ao), 1.0f);
	}
}
