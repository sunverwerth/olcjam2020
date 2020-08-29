#version 330

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

out vec2 UV;
out vec4 Color;
out vec4 gl_Position;

uniform vec2 screenSize;

void main() {
	UV = inUV;
	Color = inColor;
	gl_Position = vec4(inPosition * vec2(2, -2) / screenSize + vec2(-1, 1), 0, 1);
}