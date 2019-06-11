
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_samples;
uniform vec2 screen_size;

uniform sampler2DMS col_tex;
uniform sampler2DMS pos_tex;
uniform sampler2DMS norm_tex;

vec4 texture_ms(sampler2DMS tex, ivec2 coord) {

	vec4 color = vec4(0);

	for(int i = 0; i < num_samples; i++)
		color += texelFetch(tex, coord, i);

	return color / float(num_samples);
}

void main() {

	ivec2 coord = ivec2(f_uv * screen_size);
	
	vec3 c = texture_ms(col_tex, coord).rgb;

	color = vec4(c, 1.0f);
}

