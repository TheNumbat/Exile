
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_textures;
uniform sampler2D textures[8];
uniform float gamma;

void main() {

	//TODO(max): this is totally not the right way to blend

	vec4 col = texture(textures[0], f_uv);

	for(int i = 1; i < num_textures; i++) {

		vec4 next = texture(textures[i], f_uv);
		col = mix(col, next, next.a);
	}

	color = vec4(pow(col.rgb, vec3(1.0f / gamma)), col.a);
}

