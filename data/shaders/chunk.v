
#version 330 core

layout (location = 0) in uvec4 v_v0;
layout (location = 1) in uvec4 v_v1;

flat out uvec2 g_v0, g_v1, g_v2, g_v3;

void main() {
	g_v0 = v_v0.xy;
	g_v1 = v_v0.zw;
	g_v2 = v_v1.xy;
	g_v3 = v_v1.zw;
}
