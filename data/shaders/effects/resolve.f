
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_samples;
uniform sampler2DMS tex;
uniform vec2 screen_size;

void main() {

	ivec2 coord = ivec2(f_uv * screen_size);
	
	color = vec4(0.0f);

	for(int i = 0; i < num_samples; i++)
		color += texelFetch(tex, coord, i);

	color /= float(num_samples);
}

