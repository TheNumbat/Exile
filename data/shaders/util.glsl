
uniform float proj_a, proj_b; 

vec3 calc_pos(vec3 view, float depth) {
	float linear = proj_b / (depth - proj_a);
	return view * linear;
}

vec3 pack_norm(vec3 norm, float val) {
	vec2 xy = normalize(norm).xy;
	float sign = 2.0f * step(0.0f, norm.z) - 1.0f;
	return vec3(xy, sign * val);
}

vec3 unpack_norm(vec3 pack) {
	return vec3(pack.xy, sign(pack.z) * sqrt(1.0f - dot(pack.xy, pack.xy)));
}

vec3 scalar_to_color(float f) {
	vec3 c = vec3(4.0f * f - 2.0f, 4.0f * f + min(0.0f, 4.0f - 8.0f * f), 1.0f + 4.0f * (0.24f - f));
	return clamp(c, 0.0f, 1.0f);
}
