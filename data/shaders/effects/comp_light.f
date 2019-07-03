
#version 330 core

in vec2 f_uv;
out vec4 color;

uniform sampler2D pos_tex;
uniform sampler2D col_tex;
uniform sampler2D light_tex;

uniform float render_distance;
uniform bool sky_fog;

void main() {

	vec4 result = vec4(0.0f);

	result.xyz = texture(col_tex, f_uv).xyz * texture(light_tex, f_uv).xyz;
	vec3 pos = texture(pos_tex, f_uv).xyz;

	if(sky_fog) {
		result.w = smoothstep(1.0f, 0.9f, length(pos.xz) / render_distance);
	} else {
		result.w = 1.0f;
	}
	result.w *= 1.0f - step(length(pos), 0.0f);

	color = result;
}

