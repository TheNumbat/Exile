
#version 330 core

out vec4 color;

uniform int num_samples;
uniform int num_textures;
uniform sampler2DMS textures[8];

vec4 texture_ms(int idx, ivec2 coord) {

	vec4 color = vec4(0);

	for(int i = 0; i < num_samples; i++)
		color += texelFetch(textures[idx], coord, i);

	return color / float(num_samples);
}

void main() {

	//TODO(max): this is totally not the right way to blend

	ivec2 coord = ivec2(gl_FragCoord.xy);
	
	vec4 col = texture_ms(0, coord);

	for(int i = 1; i < num_textures; i++) {

		vec4 next = texture_ms(i, coord);
		col = mix(col, next, next.a);
	}

	color = col;
}

