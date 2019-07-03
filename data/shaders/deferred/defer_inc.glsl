
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
		l = normalize(l);
		
		vec3 h = normalize(l + v);

		float a = min(1.0f / pow(dist,3), 1.0f);

		float diff = max(dot(norm,l), 0.0f);
		light_gather += diff * f_lcol * a;
			
		float energy = (8.0f + shine) / (8.0f * PI); 
   		float spec = energy * pow(max(dot(norm, h), 0.0), shine);

		light_gather = spec * f_lcol * a;
	}

	return max(light_gather, 0.0f);
}
