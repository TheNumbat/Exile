
#version 450 core

uniform sampler2D font_texture;

in vec3 uv_f;
in vec4 color_f;

out vec4 color;

vec4 multiply_alpha(vec4 v) {
	return vec4(v.xyz * v.w, v.w);
}

vec4 divide_alpha(vec4 v) {
	return vec4(v.xyz / v.w, v.w);
}

void main() {
	
	vec4 tex = texture(font_texture, uv_f.xy);

	vec4 color_f_a = multiply_alpha(color_f);

	// we only need alpha from 
	vec4 blended = color_f_a * tex.a;

	// uf_f.z is always either zero if we are rendering geometry or one if we are rendering a texture/font
	color = mix(color_f_a, blended, uv_f.z);
}

