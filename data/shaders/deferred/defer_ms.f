
#version 330 core

#extension GL_ARB_shading_language_include : enable

in vec2 f_uv;

out vec4 color;

uniform int num_samples;
uniform vec2 screen_size;

uniform sampler2DMS col_tex;
uniform sampler2DMS pos_tex;
uniform sampler2DMS norm_tex;
uniform sampler2DMS light_tex;

uniform int debug_show;

#include </shaders/deferred/defer_inc.glsl>

vec4 get_normal(ivec2 coord, int sample, float z) {
	vec4 n = texelFetch(norm_tex, coord, sample);
	float s = sign(z);
	n.z = s * sqrt(1.0f - dot(n.xy, n.xy));
	return vec4(n.xyz, length(n.xyz) / num_samples);
}

void main() {

	ivec2 coord = ivec2(f_uv * screen_size);
	vec3 total_col = vec3(0.0f);
	float opacity = 0.0f;

	for(int i = 0; i < num_samples; i++) {
		
		vec3 col = texelFetch(col_tex, coord, i).rgb;
		vec3 light = texelFetch(light_tex, coord, i).xyz;
		vec3 pos = texelFetch(pos_tex, coord, i).xyz;
		vec4 norm = get_normal(coord, i, light.z);
	
		vec4 result = calculate_light(col, pos, light, norm);

		total_col += result.xyz;
		opacity += result.w;
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

