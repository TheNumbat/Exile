
#version 330 core

in vec2 f_uv;
out vec4 color;

uniform int num_samples;

uniform sampler2DMS col_tex;
uniform sampler2DMS light_tex;
uniform sampler2DMS depth_tex;
uniform sampler2D 	env_tex;
uniform sampler2DMS norm_tex;
uniform sampler2DMS pos_tex;

uniform float render_distance;
uniform bool sky_fog;

uniform int debug_show;

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);

	vec3 result = vec3(0.0f);
	vec3 sky = texture(env_tex, f_uv).rgb;

	for(int i = 0; i < num_samples; i++) {
		
		vec3 col = texelFetch(col_tex, coord, i).xyz * texelFetch(light_tex, coord, i).xyz;
		vec3 pos = texelFetch(pos_tex, coord, i).xyz;

		float sky_factor = step(1.0f, texelFetch(depth_tex, coord, i).x);
		if(sky_fog) {
			sky_factor += smoothstep(0.9f, 1.0f, length(pos.xz) / render_distance);
		}

		result += mix(col, sky, min(sky_factor, 1.0f));
	}

	result /= float(num_samples);

	if(debug_show == 0) {
		color = vec4(result, 1.0f);
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

