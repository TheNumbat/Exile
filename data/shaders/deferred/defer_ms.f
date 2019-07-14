
#version 400 core

#extension GL_ARB_shading_language_include : enable

flat in int instance_id;
in vec3 f_view;

out vec4 light;

uniform sampler2DMS norm_tex;
uniform sampler2DMS depth_tex;

uniform int num_instances;
uniform int debug_show;

#include </shaders/util.glsl>

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);

	int i = gl_SampleID;
			gl_SampleMask[0] = 1 << i;
	
	vec3 norm_packed = texelFetch(norm_tex, coord, i).xyz;
	float depth = texelFetch(depth_tex, coord, i).x;
	
	float shine = 1.0f / abs(norm_packed.z);
	vec3 norm = unpack_norm(norm_packed);

	vec3 pos = calc_pos(f_view, depth);
	vec3 result = calculate_light_dynamic(pos, norm, shine);

	if(debug_show == 9) {
		vec3 quad = scalar_to_color(float(instance_id) / float(num_instances));
		light = vec4(quad, 1.0f / float(num_instances));
	} else if(debug_show != 5 && debug_show != 6 && debug_show != 7) {
		light = vec4(result, 1.0f);
	}
}

