
#version 450 core

uniform sampler2D tex;

in vec3 uv_f;
in vec4 color_f;

out vec4 color;

void main() {
	
	vec4 tex = texture(tex, uv_f.xy);

	// divide out alpha
	vec4 tex_a = vec4(tex.xyz / tex.w, tex.w);

	// mix
	vec4 mixed = color_f * tex_a;

	// multiply alpha
	mixed = vec4(mixed.xyz * mixed.w, mixed.w);

	// multiply alpha
	vec4 color_f_a = vec4(color_f.xyz * color_f.w, color_f.w);

	color = mix(color_f_a, mixed, uv_f.z);
}
