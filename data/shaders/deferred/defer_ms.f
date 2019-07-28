
#version 400 core

noperspective in vec3 f_view;
out vec4 light;

uniform vec3 lcol;
uniform vec3 lpos;

uniform sampler2DMS norm_tex;
uniform sampler2DMS depth_tex;

uniform float near;
uniform int debug_show;
uniform bool dynamic_light;

uniform mat4 ivp;

const float PI = 3.14159265f;

vec3 unpack_norm(vec3 pack) {
	return vec3(pack.xy, sign(pack.z) * sqrt(1.0f - dot(pack.xy, pack.xy)));
}	

vec3 calc_pos(vec3 view, float depth) {
	return view * near / depth;
}

vec3 scalar_to_color(float f) {
	vec3 c = vec3(4.0f * f - 2.0f, 4.0f * f + min(0.0f, 4.0f - 8.0f * f), 1.0f + 4.0f * (0.24f - f));
	return clamp(c, 0.0f, 1.0f);
}

float max3(vec3 v) {
	return max(max(v.x,v.y),v.z);
}

vec3 calculate_light_dynamic(vec3 pos, vec3 norm, float shine) {

	vec3 light_gather = vec3(0.0f);

	if(dynamic_light) {
		vec3 v = normalize(-pos);
		
		vec3 l = lpos-pos;
		float dist = length(l);

		l = normalize(l);
		
		vec3 h = normalize(l + v);

		float a = min(1.0f / pow(dist,3), 1.0f);

		float diff = max(dot(norm,l), 0.0f);
		light_gather += diff * lcol * a;
			
		float energy = (8.0f + shine) / (8.0f * PI); 
   		float spec = energy * pow(max(dot(norm, h), 0.0), shine);

		light_gather += spec * lcol * a;
	} else {

		discard;
	}

	return max(light_gather, 0.0f);
}

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);

	int i = gl_SampleID;
			gl_SampleMask[0] = 1 << i;
	
	vec3 norm_packed = texelFetch(norm_tex, coord, i).xyz;
	float depth = texelFetch(depth_tex, coord, i).x;
	
	float shine = 1.0f / abs(norm_packed.z);
	vec3 norm = unpack_norm(norm_packed);

	vec3 pos = calc_pos(f_view, depth);
	vec3 result = calculate_light_dynamic(pos, norm, shine);

	if(debug_show != 5 && debug_show != 6 && debug_show != 7) {
		light = vec4(result, 1.0f);
	}
}

