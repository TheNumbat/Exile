
#version 330 core

flat in uint f_t, f_ql;
flat in vec4 f_ao;
flat in vec4 f_l;
in vec2 f_uv;
in vec3 f_n;
in float f_ah, f_d;

out vec4 out_color;

uniform sampler2DArray blocks_tex;
uniform sampler2D sky_tex;

uniform bool do_fog;
uniform bool do_ao;
uniform bool do_light;
uniform bool smooth_light;

uniform float render_distance;
uniform float day_01;
uniform float ambient;

void main() {

	vec3 uvt = vec3(f_uv, f_t);

	vec4 sample = texture(blocks_tex, uvt);

	vec3 n = normalize(f_n);

	vec3 color = sample.rgb;

	if(do_light) {

		if(smooth_light) {

			float l0 = mix(f_l.x, f_l.y, fract(f_uv.x));
			float l1 = mix(f_l.z, f_l.w, fract(f_uv.x));
			float l = mix(l0, l1, fract(f_uv.y));

			color *= clamp(ambient + l, 0.0f, 1.0f);

		} else {

			color *= clamp(ambient + (float(f_ql) / 16.0f), 0.0f, 1.0f);
		}
	}
	
	if(do_ao) {

		float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
		float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
		float ao = mix(ao0, ao1, fract(f_uv.y));

		color *= ao;
	}

	if(do_fog) {

		float fog_factor = pow(clamp(f_d / render_distance - 0.05f, 0.0f, 1.0f), 3);
		vec3 sky_color = texture(sky_tex, vec2(day_01, f_ah)).rgb;

		color = mix(color, sky_color, fog_factor);
	}

	out_color = vec4(color, sample.a);
}
