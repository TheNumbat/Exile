
#version 330 core

#extension GL_ARB_shading_language_include : enable

in vec2 f_uv;

out vec4 light;

uniform int num_samples;

uniform sampler2DMS pos_tex;
uniform sampler2DMS norm_tex;

uniform int debug_show;

flat in int instance_id;
uniform int num_instances;

#include </shaders/util.glsl>
#include </shaders/deferred/defer_inc.glsl>

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 result = vec3(0.0f);

	for(int i = 0; i < num_samples; i++) {
		
		vec3 pos = texelFetch(pos_tex, coord, i).xyz;
		vec3 norm = texelFetch(norm_tex, coord, i).xyz;
	
		float alpha = dot(norm.xy, norm.xy);
		float shine = abs(norm.z);
		norm.z = sign(norm.z) * sqrt(1.0f - alpha);
	
		result += calculate_light_dynamic(pos, norm, shine);
	}	

	light = vec4(result /= float(num_samples), 1.0f);
}

