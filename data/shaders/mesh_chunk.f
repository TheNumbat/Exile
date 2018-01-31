
#version 450 core

flat in uint f_ao, f_face, f_tex_l, f_norm;
in vec2 f_tex_uv;

out vec4 color;

uniform sampler2D tex;

void main() {

	color = texture(tex, f_tex_uv);
	color = vec4(f_tex_uv, 0.0f, 1.0f);
}
