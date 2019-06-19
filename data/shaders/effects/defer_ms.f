
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_samples;
uniform vec2 screen_size;

uniform sampler2DMS col_tex;
uniform sampler2DMS pos_tex;
uniform sampler2DMS norm_tex;
uniform sampler2DMS light_tex;

uniform int debug_show;

uniform float day_01;
uniform float ambient;
uniform float render_distance;

uniform vec3 light_col;
uniform vec3 light_pos;

uniform bool block_light;
uniform bool ambient_occlusion;
uniform bool dynamic_light;
uniform bool sky_fog;

const float PI = 3.14159265f;

vec4 get_normal(ivec2 coord, int sample, float z) {
	vec4 n = texelFetch(norm_tex, coord, sample);
	float s = sign(z);
	n.z = s * sqrt(1.0f - dot(n.xy, n.xy));
	return vec4(n.xyz, length(n.xyz));
}

void main() {

	ivec2 coord = ivec2(f_uv * screen_size);
	vec3 total_col = vec3(0.0f);
	float opacity = 0.0f;

	for(int i = 0; i < num_samples; i++) {
		
		vec3 col = texelFetch(col_tex, coord, i).rgb;
		vec3 light_vals = texelFetch(light_tex, coord, i).xyz;
		vec3 p = texelFetch(pos_tex, coord, i).xyz;
		vec4 nl = get_normal(coord, i, light_vals.z);
		vec3 n = nl.xyz;
		float op = nl.w / num_samples;

		vec3 light_gather = vec3(0.0f);

		if(block_light) {
			float day_factor = 1.0f - (smoothstep(0.32f, 0.17f, day_01) + smoothstep(0.75f, 0.9f, day_01));
			float l = max(light_vals.x, light_vals.y * day_factor);
			light_gather += vec3(pow(l,3));
		} else {
			light_gather = vec3(1.0f);
		}

		if(ambient_occlusion) {
			light_gather *= abs(light_vals.z);
		}

		if(dynamic_light) {
			vec3 v = normalize(-p);
			vec3 l = normalize(light_pos-p);
			vec3 h = normalize(l + v);

			float dist = length(light_pos-p);
			float a = min(1.0f / pow(dist,3), 1.0f);

			float diff = max(dot(n,l), 0.0f);
			light_gather += diff * light_col * a;
				
			float shine = 64.0f;
			float energy = (8.0f + shine) / (8.0f * PI); 
	   		float spec = 0.5f * energy * pow(max(dot(n, h), 0.0), shine);

			light_gather += spec * light_col * a;
		}

		if(sky_fog) {
			op *= smoothstep(1.0f, 0.9f, length(p.xz) / render_distance);
		}

		total_col += col * (ambient + light_gather);
		opacity += op;
	}	

	total_col /= float(num_samples);

	if(debug_show == 0) {	
		color = vec4(total_col, opacity);
	} else if(debug_show == 1) {
		color = vec4(vec3(pow(texelFetch(light_tex, coord, 0).x, 3)), opacity);
	} else if(debug_show == 2) {
		color = vec4(vec3(pow(texelFetch(light_tex, coord, 0).y, 3)), opacity);
	} else if(debug_show == 3) {
		color = vec4(vec3(abs(texelFetch(light_tex, coord, 0).z)), opacity);
	} else if(debug_show == 4) {
		color = vec4(texelFetch(pos_tex, coord, 0).xyz, opacity);
	} else if(debug_show == 5) {
		vec3 n = get_normal(coord, 0, texelFetch(light_tex, coord, 0).z).xyz;
		color = vec4(abs(n), opacity);
	} 
}

