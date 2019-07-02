
#version 330 core

#extension GL_ARB_shading_language_include : enable

uniform vec2 screen_size;

flat in int instance_id;
uniform int num_instances;

out vec4 color;

uniform sampler2D col_tex;
uniform sampler2D pos_tex;
uniform sampler2D norm_tex;
uniform sampler2D light_tex;

uniform int debug_show;
uniform bool base_run;

#include </shaders/util.glsl>
#include </shaders/deferred/defer_inc.glsl>

void main() {

	vec2 f_uv = gl_FragCoord.xy / screen_size;

	vec3 col = texture(col_tex, f_uv).rgb;
	vec3 light = texture(light_tex, f_uv).xyz;
	vec3 pos = texture(pos_tex, f_uv).xyz;
	
	float s = sign(light.z);

	vec3 norm = texture(norm_tex, f_uv).xyz;
	norm.z = s * sqrt(1.0f - dot(norm.xy, norm.xy));
	
	float op = calculate_fog_op(pos, length(norm));

	vec3 result;  
	if(base_run)
		result = calculate_light_base(pos, light, norm);
	else
		result = calculate_light_dynamic(pos, norm);

	result *= col;

	if(debug_show == 0) {	
		color = vec4(result, op);
	} else if(debug_show == 1) {
		color = vec4(vec3(pow(light.x, 3)), op);
	} else if(debug_show == 2) {
		color = vec4(vec3(pow(light.y, 3)), op);
	} else if(debug_show == 3) {
		color = vec4(vec3(pow(abs(light.z), 3)), op);
	} else if(debug_show == 4) {
		color = vec4(pos, op);
	} else if(debug_show == 5) {
		color = vec4(abs(norm.xyz), op);
	} else if(debug_show == 6) {
		float f = float(instance_id) / float(num_instances);
		color = vec4(scalar_to_color(f), 1.0f / float(num_instances));
	}
}

