
#version 330 core

#extension GL_ARB_shading_language_include : enable

in vec2 f_uv;

out vec4 light;

uniform int num_samples;

uniform sampler2DMS pos_tex;
uniform sampler2DMS norm_tex;

flat in int instance_id;
uniform int num_instances;
uniform int debug_show;

#include </shaders/util.glsl>
#include </shaders/deferred/defer_inc.glsl>

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 result = vec3(0.0f);

	for(int i = 0; i < num_samples; i++) {
		
		vec3 pos = texelFetch(pos_tex, coord, i).xyz;
		vec3 norm = texelFetch(norm_tex, coord, i).xyz;
	
		float alpha = dot(norm.xy, norm.xy);
		float shine = 1.0f / abs(norm.z);
		norm.z = sign(norm.z) * sqrt(1.0f - alpha);

		result += calculate_light_dynamic(pos, norm, shine);
	}	

	if(debug_show == 9) {
		light = vec4(scalar_to_color(float(instance_id) / float(num_instances)), 1.0f / float(num_instances));
	} else if(debug_show != 5 && debug_show != 6 && debug_show != 7) {
		light = vec4(result /= float(num_samples), 1.0f);
	}
}

