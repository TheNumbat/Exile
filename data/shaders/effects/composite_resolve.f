
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_samples;
uniform int num_textures;
uniform sampler2DMS textures[8];
uniform vec2 screen_size;

vec4 texture_ms(int idx, ivec2 coord) {

	vec4 color = vec4(0);

	for(int i = 0; i < num_samples; i++)
		color += texelFetch(textures[idx], coord, i);

	return color / float(num_samples);
}

void main() {

	//TODO(max): this is totally not the right way to blend

	ivec2 coord = ivec2(f_uv * screen_size);
	
	vec4 col = texture_ms(0, coord);

	for(int i = 1; i < num_textures; i++) {

		vec4 next = texture_ms(i, coord);
		col = mix(col, next, next.a);
	}

	color = col;
}

