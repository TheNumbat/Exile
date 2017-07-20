
#version 450 core

uniform sampler2D font_texture;

in vec3 uv_f;
in vec4 color_f;

out vec4 color;

void main() {
	
	vec4 tex = texture(font_texture, uv_f.xy);

	vec4 color_f_a = vec4(color_f.rgb * color_f.a, color_f.a);

	color = mix(color_f_a, tex, uv_f.z);
}
