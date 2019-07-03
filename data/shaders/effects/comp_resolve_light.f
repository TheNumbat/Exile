
#version 330 core

out vec4 color;

uniform int num_samples;

uniform sampler2DMS pos_tex;
uniform sampler2DMS col_tex;
uniform sampler2DMS light_tex;
uniform sampler2DMS norm_tex;

uniform float render_distance;
uniform bool sky_fog;

uniform int debug_show;

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);

	vec4 result = vec4(0.0f);

	for(int i = 0; i < num_samples; i++) {
		
		result.xyz += texelFetch(col_tex, coord, i).xyz * texelFetch(light_tex, coord, i).xyz;
		vec3 pos = texelFetch(pos_tex, coord, i).xyz;

		if(sky_fog) {
			result.w += smoothstep(1.0f, 0.9f, length(pos.xz) / render_distance);
		} else {
			result.w += 1.0f;
		}
		result.w *= 1.0f - step(length(pos), 0.0f);
	}

	result /= float(num_samples);

	if(debug_show == 0) {
		color = result;
	} else if(debug_show == 1) {
		color = vec4(texelFetch(col_tex, coord, 0).xyz, 1.0f);
	} else if(debug_show == 2) {
		color = vec4(texelFetch(pos_tex, coord, 0).xyz, 1.0f);
	} else if(debug_show == 3) {
		vec3 norm = texelFetch(norm_tex, coord, 0).xyz;
		norm.z = sign(norm.z) * sqrt(1.0f - dot(norm.xy, norm.xy));
		color = vec4(abs(norm), 1.0f);
	} else {
		color = vec4(texelFetch(light_tex, coord, 0).xyz, 1.0f);
	}
}

