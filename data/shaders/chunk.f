
#version 330 core

flat in uint f_t, f_ql, f_qs;
flat in vec4 f_ao;
flat in vec4 f_l, f_s;
in vec2 f_uv;
in vec3 f_n;
in float f_ah, f_d;
in vec3 f_pos;

layout (location = 0) out vec4 out_color;

uniform sampler2DArray blocks_tex;
uniform sampler2D sky_tex;

uniform bool do_fog;
uniform bool do_light;
uniform bool smooth_light;
uniform bool dynamic_light;

uniform vec3 light_col;
uniform vec3 light_pos;
uniform int debug_light;

uniform float render_distance;
uniform float day_01;
uniform float ambient;

const float PI = 3.14159265f;

void main() {

	if(debug_light == 1) {
		float t = float(f_ql) / 15.0f;
		out_color = vec4(clamp(vec3(t), 0.0f, 1.0f), 1.0f);
		return;
	} else if(debug_light == 2) {
		float s = float(f_qs) / 15.0f;
		out_color = vec4(clamp(vec3(s), 0.0f, 1.0f), 1.0f);
		return;
	} else if(debug_light == 3) {
		out_color = vec4(f_pos, 1.0f);
		return;
	} else if(debug_light == 4) {
		out_color = vec4(abs(normalize(f_n)), 1.0f);
		return;
	}

	vec3 uvt = vec3(f_uv, f_t);
	vec3 color = texture(blocks_tex, uvt).rgb;

	vec3 total_light = vec3(0.0f);
	float ao = 1.0f;

	if(do_light) {

		float day_factor = 1.0f - (smoothstep(0.32f, 0.17f, day_01) + smoothstep(0.75f, 0.9f, day_01));

		float ao0 = mix(f_ao.x, f_ao.y, fract(f_uv.x));
		float ao1 = mix(f_ao.z, f_ao.w, fract(f_uv.x));
		ao = mix(ao0, ao1, fract(f_uv.y));

		if(smooth_light) {

			float t0 = mix(f_l.x, f_l.y, fract(f_uv.x));
			float t1 = mix(f_l.z, f_l.w, fract(f_uv.x));
			float t = mix(t0, t1, fract(f_uv.y));

			float s0 = mix(f_s.x, f_s.y, fract(f_uv.x));
			float s1 = mix(f_s.z, f_s.w, fract(f_uv.x));
			float s = mix(s0, s1, fract(f_uv.y)) * day_factor;

			float l = max(t,s);
			total_light += vec3(pow(l,3));

		} else {

			float t = float(f_ql) / 15.0f;
			float s = float(f_qs) / 15.0f * day_factor;
			float l = max(t,s);
			total_light += vec3(pow(l,3));
		}
	}

	total_light *= ao;

	if(dynamic_light) {
		vec3 p = f_pos;
		vec3 n = normalize(f_n);
		vec3 v = normalize(-p);
		vec3 l = normalize(light_pos-p);
		vec3 h = normalize(l + v);

		float dist = length(light_pos-p);
		float a = min(1.0f / (dist * dist), 1.0f);

		float diff = max(dot(n,l), 0.0f);
		total_light += diff * light_col * a;
			
		float shine = 32.0f;
		float energy = (8.0f + shine) / (8.0f * PI); 
   		float spec = 0.5f * energy * pow(max(dot(n, h), 0.0), shine);

		total_light += spec * light_col * a;
	}

	color *= ambient + total_light;

	if(do_fog) {

		float fog_factor = smoothstep(0.9f, 1.0f, f_d / render_distance);
		vec3 sky_color = texture(sky_tex, vec2(day_01, f_ah)).rgb;

		color = mix(color, sky_color, fog_factor);
	}

	out_color = vec4(color, 1.0f);
}
