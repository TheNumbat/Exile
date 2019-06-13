
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_samples;
uniform vec2 screen_size;

uniform sampler2DMS col_tex;
uniform sampler2DMS pos_tex;
uniform sampler2DMS norm_tex;
uniform sampler2DMS coverage_tex;

uniform int debug_show;

vec4 texture_ms(sampler2DMS tex, ivec2 coord) {

	vec4 color = vec4(0);

	for(int i = 0; i < num_samples; i++)
		color += texelFetch(tex, coord, i);

	return color / float(num_samples);
}

void main() {

	ivec2 coord = ivec2(f_uv * screen_size);
	
	vec3 c = texture_ms(col_tex, coord).rgb;
	float cov = texelFetch(coverage_tex, coord, 0).r;

	if(debug_show == 0) {	
		color = vec4(c, 1.0f);
	} else if(debug_show == 1) {
		color = vec4(texelFetch(pos_tex, coord, 0).rgb, 1.0f);
	} else if(debug_show == 2) {
		color = vec4(abs(texelFetch(norm_tex, coord, 0).rgb), 1.0f);
	} else {
		color = cov == 0.0f ? vec4(0.0f, 0.0f, 0.0f, 1.0f) : vec4(1.0f);
	}
}

