
uniform float day_01;
uniform float ambient;
uniform float render_distance;

uniform vec3 light_col;
uniform vec3 light_pos;

uniform bool block_light;
uniform bool ambient_occlusion;
uniform bool dynamic_light;
uniform bool sky_fog;

const float PI = 3.14159265f;

float calculate_fog_op(vec3 pos, float op) {

	if(sky_fog) {
		op *= smoothstep(1.0f, 0.9f, length(pos.xz) / render_distance);
	}

	return op;
}

vec3 calculate_light_base(vec3 pos, vec3 light, vec3 norm) {

	vec3 n = norm.xyz;

	vec3 light_gather = vec3(0.0f);

	if(block_light) {
		float day_factor = 1.0f - (smoothstep(0.32f, 0.17f, day_01) + smoothstep(0.75f, 0.9f, day_01));
		float l = max(light.x, light.y * day_factor);
		light_gather += vec3(pow(l,3));
	} else {
		light_gather = vec3(0.0f);
	}

	if(ambient_occlusion) {
		light_gather *= abs(light.z);
	}

	return ambient + light_gather;
}

vec3 calculate_light_dynamic(vec3 pos, vec3 norm) {

	vec3 light_gather = vec3(0.0f);

	if(dynamic_light) {
		vec3 v = normalize(-pos);
		
		vec3 l = light_pos-pos;
		float dist = length(l);
		l = normalize(l);
		
		vec3 h = normalize(l + v);

		float a = min(1.0f / pow(dist,3), 1.0f);

		float diff = max(dot(norm,l), 0.0f);
		light_gather += diff * light_col * a;
			
		float shine = 64.0f;
		float energy = (8.0f + shine) / (8.0f * PI); 
   		float spec = energy * pow(max(dot(norm, h), 0.0), shine);

		light_gather = spec * light_col * a;
	}

	return light_gather;
}
