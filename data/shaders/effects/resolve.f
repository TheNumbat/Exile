
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_samples;
uniform sampler2DMS tex;

void main() {

	ivec2 coord = ivec2(gl_FragCoord);
	
	color = vec4(0.0f);

	for(int i = 0; i < num_samples; i++)
		color += texelFetch(tex, coord, i);

	color /= float(num_samples);
}

