
#version 330 core

in vec2 f_uv;
in vec3 f_view;
out vec4 color;

uniform sampler2D col_tex;
uniform sampler2D light_tex;
uniform sampler2D depth_tex;
uniform sampler2D env_tex;
uniform sampler2D norm_tex;

uniform float render_distance;
uniform float near;
uniform bool sky_fog;

uniform int debug_show;

vec3 calc_pos(vec3 view, float depth) {
	return view * near / depth;
}

void main() {

	vec3 light = texture(light_tex, f_uv).xyz;
	vec3 col = texture(col_tex, f_uv).rgb;
	vec3 sky = texture(env_tex, f_uv).rgb;
	float depth = texture(depth_tex, f_uv).x;
	vec3 pos = calc_pos(f_view, depth);

	float sky_factor = depth == 0.0f ? 1.0f : 0.0f;
	if(sky_fog) {
		sky_factor += smoothstep(0.9f, 1.0f, length(pos.xz) / render_distance);
	}

	vec3 result = mix(col * light, sky, min(sky_factor, 1.0f));

	if(debug_show == 0) {
		color = vec4(result, 1.0f);
	} else if(debug_show == 1) {
		color = vec4(col, 1.0f);
	} else if(debug_show == 2) {
		color = vec4(pos, 1.0f);
	} else if(debug_show == 3) {
		vec3 norm = texture(norm_tex, f_uv).xyz;
		norm.z = sign(norm.z) * sqrt(1.0f - dot(norm.xy, norm.xy));
		color = vec4(abs(norm), 1.0f);
	} else if(debug_show == 10) {
		color = vec4(vec3(sqrt(texture(depth_tex, f_uv).x)), 1.0f);
	} else {
		color = vec4(light, 1.0f);
	}
}

