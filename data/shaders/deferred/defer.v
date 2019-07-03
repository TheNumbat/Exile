
#version 330 core

layout (location = 0) in vec4 quad;

layout (location = 1) in vec3 v_lpos;
layout (location = 2) in vec3 v_lcol;

out vec2 f_uv;
flat out vec3 f_lpos;
flat out vec3 f_lcol;
flat out int instance_id;

void main() {
	
	vec2 pos[4] = vec2[4](quad.xy, 
						  quad.xy + vec2(quad.z, 0.0f),
						  quad.xy + vec2(0.0f, quad.w),
						  quad.xy + quad.zw);

	gl_Position = vec4(pos[gl_VertexID], 0.0f, 1.0f);

	f_uv = pos[gl_VertexID] / 2.0f + vec2(0.5f);

	f_lpos = v_lpos;
	f_lcol = v_lcol;
	instance_id = gl_InstanceID;
}
