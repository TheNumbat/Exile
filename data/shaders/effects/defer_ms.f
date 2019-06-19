
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_samples;
uniform vec2 screen_size;

uniform sampler2DMS col_tex;
uniform sampler2DMS pos_tex;
uniform sampler2DMS norm_tex;
uniform sampler2DMS light_tex;
uniform sampler2D sky_tex;

uniform int debug_show;

vec3 get_normal(ivec2 coord, int sample) {
	vec4 n = texelFetch(norm_tex, coord, sample);
	float s = sign(texelFetch(light_tex, coord, sample).z);
	n.z = s * sqrt(1.0f - dot(n.xy, n.xy));
	return normalize(n.xyz);
}

vec3 get_pos(ivec2 coord, int sample) {
	return texelFetch(pos_tex, coord, sample).xyz;
}

void main() {

	ivec2 coord = ivec2(f_uv * screen_size);
	vec3 total_col = vec3(0.0f);
	vec3 total_light = vec3(0.0f);

	for(int i = 0; i < num_samples; i++) {
		
		vec3 col = texelFetch(col_tex, coord, i).rgb;
		vec3 light = texelFetch(light_tex, coord, i).xyz;

		total_col += col;
		total_light += abs(light);
	}	

	total_col /= float(num_samples);
	total_light /= float(num_samples);

	if(debug_show == 0) {	
		color = vec4(total_col, 1.0f);
	} else if(debug_show == 1) {
		color = vec4(vec3(total_light.x), 1.0f);
	} else if(debug_show == 2) {
		color = vec4(vec3(total_light.y), 1.0f);
	} else if(debug_show == 3) {
		color = vec4(vec3(total_light.z), 1.0f);
	} else if(debug_show == 4) {
		color = vec4(get_pos(coord, 0), 1.0f);
	} else if(debug_show == 5) {
		vec3 n = get_normal(coord, 0);
		color = vec4(abs(n), 1.0f);
	} 
}

