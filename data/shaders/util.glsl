
uniform float near; 

vec3 calc_pos(vec3 view, float depth) {
	return view * near / depth;
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

float max3(vec3 v) {
	return max(max(v.x,v.y),v.z);
}

flat in vec3 f_lcol;
flat in vec3 f_lpos;

uniform bool dynamic_light;

const float PI = 3.14159265f;

vec3 calculate_light_dynamic(vec3 pos, vec3 norm, float shine) {

	vec3 light_gather = vec3(0.0f);

	if(dynamic_light) {
		vec3 v = normalize(-pos);
		
		vec3 l = f_lpos-pos;
		float dist = length(l);

		if(dist > 8.0f * max3(f_lcol)) {
			discard;
		}

		l = normalize(l);
		
		vec3 h = normalize(l + v);

		float a = min(1.0f / pow(dist,3), 1.0f);

		float diff = max(dot(norm,l), 0.0f);
		light_gather += diff * f_lcol * a;
			
		float energy = (8.0f + shine) / (8.0f * PI); 
   		float spec = energy * pow(max(dot(norm, h), 0.0), shine);

		light_gather += spec * f_lcol * a;
	} else {

		discard;
	}

	return max(light_gather, 0.0f);
}
