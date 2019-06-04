
#version 330 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec2 v_tc;

uniform float day_01;
uniform mat4 transform;

out vec2 f_tc;

void main() {
	
	vec4 pos = transform * vec4(v_pos, 1.0);
	f_tc = v_tc;
	
	gl_Position = pos.xyww;
}
