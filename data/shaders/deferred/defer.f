
#version 330 core

#extension GL_ARB_shading_language_include : enable

flat in int instance_id;
in vec2 f_uv;
in vec3 f_view;

out vec4 light;

uniform sampler2D norm_tex;
uniform sampler2D depth_tex;

uniform int num_instances;
uniform int debug_show;

#include </shaders/util.glsl>

void main() {

	vec3 norm_packed = texture(norm_tex, f_uv).xyz;
	float depth = texture(depth_tex, f_uv).x;
	
	float shine = 1.0f / abs(norm_packed.z);
	vec3 norm = unpack_norm(norm_packed);

	vec3 pos = calc_pos(f_view, depth);
	vec3 result = calculate_light_dynamic(pos, norm, shine);

	if(debug_show == 9) {
		vec3 quad = scalar_to_color(float(instance_id) / float(num_instances));
		light = vec4(result * quad, 1.0f / float(num_instances));
	} else if(debug_show != 5 && debug_show != 6 && debug_show != 7) {
		light = vec4(result, 1.0f);
	} 
}

