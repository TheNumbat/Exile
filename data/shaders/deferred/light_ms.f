
#version 400 core

noperspective in vec3 f_view;

flat in vec3 f_lpos, f_ldir, f_ldiff, f_lspec, f_lattn;
flat in vec2 f_lcut;
flat in float f_r;
flat in vec3 instance_col;

out vec4 light;

uniform sampler2DMS norm_tex;
uniform sampler2DMS depth_tex;

uniform float near;
uniform int debug_show;

uniform mat4 ivp;

const float PI = 3.14159265f;

vec3 unpack_norm(vec3 pack) {
	return vec3(pack.xy, sign(pack.z) * sqrt(1.0f - dot(pack.xy, pack.xy)));
}	

vec3 calc_pos(vec3 view, float depth) {
	return view * near / depth;
}

vec3 calculate_light_dynamic(vec3 pos, vec3 norm, float shine) {

	vec3 light_gather = vec3(0.0f);

	vec3 v = normalize(-pos);
	vec3 l = f_lpos-pos;

	float dist = length(l);
	l = normalize(l);

	vec3 h = normalize(l + v);

	float a = 1.0f / (f_lattn.x + f_lattn.y * dist + f_lattn.z * dist * dist);
	
	if(length(f_ldir) > 0.0f) {
		a *= smoothstep(f_lcut.y, f_lcut.x, dot(l,-f_ldir));
	}

	float diff = max(dot(norm,l), 0.0f);
	light_gather += max(diff * f_ldiff * a, 0.0f);
		
	float energy = (8.0f + shine) / (8.0f * PI); 
	float spec = 2.0f * energy * pow(max(dot(norm, h), 0.0), shine);

	light_gather += max(spec * f_lspec * a, 0.0f);

	return light_gather;	
}

void main() {

	ivec2 coord = ivec2(gl_FragCoord.xy);
	int i = gl_SampleID;

	float depth = texelFetch(depth_tex, coord, i).x;

	vec3 pos = calc_pos(f_view, depth);
	if(length(pos - f_lpos) > f_r) discard;

	gl_SampleMask[0] = 1 << i;
	
	vec3 norm_packed = texelFetch(norm_tex, coord, i).xyz;
	float shine = 1.0f / abs(norm_packed.z);
	vec3 norm = unpack_norm(norm_packed);

	vec3 result = calculate_light_dynamic(pos, norm, shine);

	if(debug_show == 10) {
		light = vec4(instance_col, 1.0f);
	} else if(debug_show != 5 && debug_show != 6 && debug_show != 7) {
		light = vec4(result, 1.0f);
	} 
}

