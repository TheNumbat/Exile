
#version 330 core

flat in uint f_t, f_ql, f_qs;
flat in vec4 f_ao;
flat in vec4 f_l, f_s;
in vec2 f_uv;
in vec3 f_n;
in float f_ah, f_d;

out vec4 out_color;

uniform sampler2DArray blocks_tex;
uniform sampler2D sky_tex;

uniform bool do_fog;
uniform bool do_light;
uniform bool smooth_light;
uniform int debug_light;

uniform float render_distance;
uniform float day_01;
uniform float ambient;

void main() {

	vec3 uvt = vec3(f_uv, f_t);

	vec4 sample = texture(blocks_tex, uvt);

	vec3 n = normalize(f_n);

	vec3 color = sample.rgb;

	if(do_light) {

		float day_factor = 1.0f - (smoothstep(0.32f, 0.17f, day_01) + smoothstep(0.75f, 0.9f, day_01));

		if(debug_light == 1) {

			float t = float(f_ql) / 15.0f;
			color = clamp(vec3(t), 0.0f, 1.0f);

		} else if(debug_light == 2) {

			float s = float(f_qs) / 15.0f;
			color = clamp(vec3(s), 0.0f, 1.0f);

		} else if(smooth_light) {

			float t0 = mix(f_l.x, f_l.y, fract(f_uv.x));
			float t1 = mix(f_l.z, f_l.w, fract(f_uv.x));
			float t = mix(t0, t1, fract(f_uv.y));

			float s0 = mix(f_s.x, f_s.y, fract(f_uv.x));
			float s1 = mix(f_s.z, f_s.w, fract(f_uv.x));
			float s = mix(s0, s1, fract(f_uv.y)) * day_factor;

			float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
			float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
			float ao = mix(ao0, ao1, fract(f_uv.y));

			float l = max(t,s);
			color *= clamp(ambient + ao * l * l, 0.0f, 1.0f);

		} else {

			float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
			float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
			float ao = mix(ao0, ao1, fract(f_uv.y));

			float t = float(f_ql) / 15.0f;
			float s = float(f_qs) / 15.0f * day_factor;

			float l = max(t,s);
			color *= clamp(ambient + ao * l * l, 0.0f, 1.0f);
		}
	}

	if(do_fog) {

		float fog_factor = pow(clamp(f_d / render_distance - 0.05f, 0.0f, 1.0f), 3);
		vec3 sky_color = texture(sky_tex, vec2(day_01, f_ah)).rgb;

		color = mix(color, sky_color, fog_factor);
	}

	out_color = vec4(color, sample.a);
}
