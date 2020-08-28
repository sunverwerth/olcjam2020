#version 330

in UV;
in Color;

uniform sampler2D mainTexture

void main() {
	gl_Fragcolor = texture(mainTexture, UV) * Color;
}