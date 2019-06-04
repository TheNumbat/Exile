
#version 330 core

layout (location = 0) in vec4 v_pos;

uniform float day_01;
uniform mat4 transform;

void main() {
	
	vec4 pos = transform * vec4(v_pos.xyz, 1.0);
	
	gl_Position = pos.xyww;
	gl_PointSize = abs(v_pos.w * sin((day_01 + v_pos.w) * 50.0f * 3.14159f));
}
