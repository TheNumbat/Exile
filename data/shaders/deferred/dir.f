
#version 330 core

uniform vec3 ldir;
uniform vec3 ldiff;
uniform vec3 lspec;
uniform float near;

in vec2 f_uv;
in vec3 f_view;
out vec4 light;

uniform sampler2D norm_tex;
uniform sampler2D depth_tex;

const float PI = 3.14159265f;

vec3 calc_pos(vec3 view, float depth) {
	return view * near / depth;
}

vec3 unpack_norm(vec3 pack) {
	return vec3(pack.xy, sign(pack.z) * sqrt(1.0f - dot(pack.xy, pack.xy)));
}	

vec3 calculate_light_dynamic(vec3 pos, vec3 norm, float shine) {

	vec3 light_gather = vec3(0.0f);

	vec3 v = normalize(-pos);
	vec3 l = normalize(-ldir);
	vec3 h = normalize(l + v);

	float diff = max(dot(norm,l), 0.0f);
	light_gather += diff * ldiff;
		
	float energy = (8.0f + shine) / (8.0f * PI); 
		float spec = energy * pow(max(dot(norm, h), 0.0), shine);

	light_gather += spec * lspec;

	return max(light_gather, 0.0f);
}

void main() {

	float depth = texture(depth_tex, f_uv).x;

	vec3 pos = calc_pos(f_view, depth);

	vec3 norm_packed = texture(norm_tex, f_uv).xyz;
	
	float shine = 1.0f / abs(norm_packed.z);
	vec3 norm = unpack_norm(norm_packed);

	vec3 result = calculate_light_dynamic(pos, norm, shine);

	light = vec4(result, 1.0f);
}
