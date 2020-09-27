/*
MIT License

Copyright(c) 2020 Stephan Unverwerth

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Texture.h"
#include "glad.h"
#include "Image.h"
#include "sys.h"

Texture::Texture(const Image& image): width_(image.width()), height_(image.height()) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, image.format() == Image::Format::RGB8 ? GL_RGB8 : GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	load(image);
}

Texture::~Texture() {
	glDeleteTextures(1, &texture);
}

void Texture::load(const Image& image) {
	if (image.width() != width_ || image.height() != height_) {
		log_error("Texture can only be updated with same size image.");
		return;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, image.format() == Image::Format::RGB8 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, image.data());
}

void Texture::bind(int unit) {
	if (unit_ >= 0) unbind();
	unit_ = unit;
	glActiveTexture(GL_TEXTURE0 + unit_);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void Texture::unbind() {
	if (unit_ < 0) return;
	glActiveTexture(GL_TEXTURE0 + unit_);
	glBindTexture(GL_TEXTURE_2D, 0);
	unit_ = -1;
}