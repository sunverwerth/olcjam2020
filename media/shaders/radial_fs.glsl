#version 330

in vec2 UV;
in vec4 Color;

out vec4 outColor;

uniform float progress;

const float PI = 3.141592653;

void main() {
	float angle = atan(UV.x - 0.5, 0.5 - UV.y);
	outColor = min(Color, 1) + max(Color - 1, 0);
	outColor.a *= (-PI + progress * PI * 2 > angle) ? 1 : 0;
}