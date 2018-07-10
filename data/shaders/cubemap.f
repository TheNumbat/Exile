
#version 330

smooth in vec3 f_texcoord;

out vec4 color;

uniform samplerCube tex;

void main() {
	color = texture(tex, f_texcoord);
}
