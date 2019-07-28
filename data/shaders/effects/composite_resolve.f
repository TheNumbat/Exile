
#version 330 core

out vec4 color;

uniform int num_samples;
uniform int num_textures;
uniform sampler2DMS textures[8];

vec4 texture_ms(sampler2DMS tex, ivec2 coord) {

	vec4 color = vec4(0);

	for(int i = 0; i < num_samples; i++)
		color += texelFetch(tex, coord, i);

	return color / float(num_samples);
}

#define blend(i) {vec4 next = texture_ms(textures[i], coord); col = mix(col, next, next.a);}

void main() {

	//TODO(max): this is totally not the right way to blend

	ivec2 coord = ivec2(gl_FragCoord.xy);
	
	vec4 col = texture_ms(textures[0], coord);

	switch(num_textures){
	case 8: blend(1);
	case 7: blend(2);
	case 6: blend(3);
	case 5: blend(4);
	case 4: blend(5);
	case 3: blend(6);
	case 2: blend(7);
	default: break;
	}

	color = col;
}

