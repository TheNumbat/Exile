
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_samples;
uniform vec2 screen_size;

uniform sampler2DMS col_tex;
uniform sampler2DMS pos_tex;
uniform sampler2DMS norm_tex;

uniform int debug_show;
uniform bool do_light;

uniform vec3 light_col;

void main() {

	ivec2 coord = ivec2(f_uv * screen_size);
	
	vec3 total = vec3(0);

	for(int i = 0; i < num_samples; i++) {
		
		vec3 col = texelFetch(col_tex, coord, i).rgb;

		if (do_light) {
			vec3 p = texelFetch(pos_tex, coord, i).rgb;
			vec3 n = texelFetch(norm_tex, coord, i).rgb;
			vec3 l = normalize(-p);
			float dist = length(p);

			float diff = max(dot(n,l), 0.0f);
			col *= diff * light_col * (1.0f / (dist * dist));
		}

		total += col;
	}	

	if(debug_show == 0) {	
		color = vec4(total / float(num_samples), 1.0f);
	} else if(debug_show == 1) {
		color = vec4(texelFetch(pos_tex, coord, 0).rgb, 1.0f);
	} else {
		color = vec4(abs(texelFetch(norm_tex, coord, 0).rgb), 1.0f);
	} 
}

