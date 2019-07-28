
#version 330 core

in vec2 f_uv;

out vec4 color;

uniform int num_textures;
uniform sampler2D textures[8];

#define blend(i) {vec4 next = texture(textures[i], f_uv); col = mix(col, next, next.a);}

void main() {

	vec4 col = texture(textures[0], f_uv);

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

