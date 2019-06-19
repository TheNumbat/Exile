
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform sampler2D col_tex;
uniform sampler2D pos_tex;
uniform sampler2D norm_tex;
uniform sampler2D light_tex;

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

void main() {

	vec3 col = texture(col_tex, f_uv).rgb;
	vec3 light_vals = texture(light_tex, f_uv).xyz;

	vec3 p = texture(pos_tex, f_uv).xyz;
	
	vec3 n = texture(norm_tex, f_uv).xyz;
	float s = sign(light_vals.z);
	n.z = s * sqrt(1.0f - dot(n.xy, n.xy));
	float opacity = length(n);

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
		opacity *= smoothstep(1.0f, 0.9f, length(p.xz) / render_distance);
	}

	col *= ambient + light_gather;

	if(debug_show == 0) {	
		color = vec4(col, opacity);
	} else if(debug_show == 1) {
		color = vec4(vec3(pow(light_vals.x, 3)), opacity);
	} else if(debug_show == 2) {
		color = vec4(vec3(pow(light_vals.y, 3)), opacity);
	} else if(debug_show == 3) {
		color = vec4(vec3(pow(abs(light_vals.z), 3)), opacity);
	} else if(debug_show == 4) {
		color = vec4(p, opacity);
	} else if(debug_show == 5) {
		color = vec4(abs(n), opacity);
	} 
}

