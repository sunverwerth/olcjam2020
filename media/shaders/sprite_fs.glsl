#version 330

in vec2 UV;
in vec4 Color;

out vec4 outColor;

uniform sampler2D mainTexture;

void main() {
	outColor = texture(mainTexture, UV) * min(Color, 1) + max(Color - 1, 0);
}