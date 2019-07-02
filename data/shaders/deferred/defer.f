
#version 330 core

#extension GL_ARB_shading_language_include : enable

in vec2 f_uv;

out vec4 color;

uniform sampler2D col_tex;
uniform sampler2D pos_tex;
uniform sampler2D norm_tex;
uniform sampler2D light_tex;

uniform int debug_show;

#include </shaders/deferred/defer_inc.glsl>

void main() {

	vec3 col = texture(col_tex, f_uv).rgb;
	vec3 light = texture(light_tex, f_uv).xyz;
	vec3 pos = texture(pos_tex, f_uv).xyz;
	
	float s = sign(light.z);

	vec4 norm = texture(norm_tex, f_uv);
	norm.z = s * sqrt(1.0f - dot(norm.xy, norm.xy));
	norm.w = length(norm.xyz);

	vec4 result = calculate_light(col, pos, light, norm);

	if(debug_show == 0) {	
		color = vec4(result);
	} else if(debug_show == 1) {
		color = vec4(vec3(pow(light.x, 3)), result.w);
	} else if(debug_show == 2) {
		color = vec4(vec3(pow(light.y, 3)), result.w);
	} else if(debug_show == 3) {
		color = vec4(vec3(pow(abs(light.z), 3)), result.w);
	} else if(debug_show == 4) {
		color = vec4(pos, result.w);
	} else if(debug_show == 5) {
		color = vec4(abs(norm.xyz), result.w);
	} 
}

