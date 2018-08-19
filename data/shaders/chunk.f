
#version 330 core

flat in uint f_t;
flat in vec4 f_ao;
in vec2 f_uv;
in vec3 f_n;
in float f_ah, f_d;

out vec4 out_color;

uniform sampler2DArray blocks_tex;
uniform sampler2D sky_tex;

uniform bool do_fog;
uniform bool do_ao;

uniform float render_distance;
uniform float day_01;

void main() {

	vec3 uvt = vec3(f_uv, f_t);

	vec4 sample = texture(blocks_tex, uvt);

	vec3 n = normalize(f_n);

	vec3 color = sample.rgb;

	if(do_ao) {

		float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
		float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
		float ao = mix(ao0, ao1, fract(f_uv.y));

		color *= ao;
	}

	if(do_fog) {

		float fog_factor = clamp(f_d / (1.25f * render_distance), 0.0f, 1.0f);

		vec3 sky_color = texture(sky_tex, vec2(day_01, f_ah)).rgb;

		color = mix(color, sky_color, fog_factor);
	}

	out_color = vec4(color, sample.a);
}
