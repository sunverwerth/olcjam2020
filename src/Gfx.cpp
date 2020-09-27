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

#include "Gfx.h"
#include "sys.h"
#include "Sprite.h"
#include "Shader.h"
#include "Texture.h"
#include "glad.h"
#include "Image.h"
#include "Mesh.h"

void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if (type == GL_DEBUG_TYPE_ERROR) log_error(message);
	else log(message);
}

Gfx::Gfx(const char* title, int width, int height, bool fullscreen) {
	log("Gfx::gfx()");
	int windowFlags = SDL_WINDOW_OPENGL;
	if (fullscreen) windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	else windowFlags |= SDL_WINDOW_RESIZABLE;
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, windowFlags);
	if (!window) sys_crash("Could not create SDL window.");
	SDL_GetWindowSize(window, &width_, &height_);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, SDL_TRUE);
	unsigned int flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
#ifdef _DEBUG
	flags |= GL_CONTEXT_FLAG_DEBUG_BIT;
#endif
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, flags);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, GL_CONTEXT_CORE_PROFILE_BIT);

	context = SDL_GL_CreateContext(window);
	if (!context) sys_crash("Could not create GL context.");

	if (SDL_GL_MakeCurrent(window, context)) sys_crash("Could not make context current.");

	if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) sys_crash("Could not load GL functions.");

#ifdef _DEBUG
	if (glDebugMessageCallback) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debugCallback, nullptr);
	}
#endif

	int numTextureUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &numTextureUnits);
	textureUnits.resize(numTextureUnits);

	spriteShader = new Shader("media/shaders/sprite_vs.glsl", "media/shaders/sprite_fs.glsl");
	radialProgressShader = new Shader("media/shaders/sprite_vs.glsl", "media/shaders/radial_fs.glsl");

	spriteMesh = new Mesh();
}

Gfx::~Gfx() {
	delete spriteMesh;
	delete spriteShader;
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
}

void Gfx::beginFrame() {
	SDL_GetWindowSize(window, &width_, &height_);

	glViewport(0, 0, width_, height_);
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Gfx::endFrame() {
	endSprites();
	SDL_GL_SwapWindow(window);
}

void Gfx::bindTexture(Texture* texture) {
	if (texture->unit() >= 0) return;
	for (int i = 0; i < textureUnits.size(); i++) {
		if (textureUnits[i] == nullptr) {
			texture->bind(i);
			textureUnits[i] = texture;
			return;
		}
	}
}

Texture* Gfx::getTexture(const char* name) {
	auto it = loadedTextures.find(name);
	if (it != loadedTextures.end()) return it->second;

	auto image = Image(name);
	auto texture = new Texture(image);
	loadedTextures[name] = texture;
	return texture;
}

void Gfx::beginSprites(Texture* texture) {
	if (currentSpriteTexture == texture) return;
	if (currentSpriteTexture) endSprites();
	spriteVertices.clear();
	currentSpriteTexture = texture;
}

void Gfx::endSprites() {
	if (!spriteVertices.empty()) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		spriteShader->use();
		bindTexture(currentSpriteTexture);
		spriteShader->texture("mainTexture", *currentSpriteTexture);
		spriteShader->uniform("screenSize", Vec2(width_, height_));
		spriteMesh->setVertices(spriteVertices.data(), sizeof(SpriteVertex), spriteVertices.size());
		spriteMesh->bind();
		glDrawElements(GL_TRIANGLES, spriteMesh->numVertices() * 6 / 4, GL_UNSIGNED_SHORT, nullptr);
	}
	currentSpriteTexture = nullptr;
}

void Gfx::drawTexture(Texture* texture, const Vec2& pos, const Vec4& color) {
	if (currentSpriteTexture != texture) beginSprites(texture);
	const float w = texture->width() * pixelScale;
	const float h = texture->height() * pixelScale;
	spriteVertices.push_back({ pos * pixelScale, {0, 1}, color });
	spriteVertices.push_back({ pos * pixelScale + Vec2(w, 0), {1, 1}, color });
	spriteVertices.push_back({ pos * pixelScale + Vec2(w, h), {1, 0}, color });
	spriteVertices.push_back({ pos * pixelScale + Vec2(0, h), {0, 0}, color });
}

void Gfx::drawTextureClip(Texture* texture, const Vec2& clipPos, const Vec2& clipSize, const Vec2& pos, const Vec2& size, const Vec4& color, bool mirrored) {
	if (currentSpriteTexture != texture) beginSprites(texture);
	auto uv = clipPos / Vec2(texture->width(), texture->height());
	uv.y = 1 - uv.y;
	auto du = clipSize.x / texture->width();
	auto dv = -clipSize.y / texture->height();
	if (mirrored) {
		uv.x += du;
		du *= -1;
	}
	const float w = size.x * pixelScale;
	const float h = size.y * pixelScale;
	spriteVertices.push_back({ pos * pixelScale, uv, color });
	spriteVertices.push_back({ pos * pixelScale + Vec2(w, 0), uv + Vec2(du, 0), color });
	spriteVertices.push_back({ pos * pixelScale + Vec2(w, h), uv + Vec2(du, dv), color });
	spriteVertices.push_back({ pos * pixelScale + Vec2(0, h), uv + Vec2(0, dv), color });
}

void Gfx::drawTextureSliced(Texture* texture, const Vec2& clipPos, const Vec2& clipSize, const Vec4& borders, const Vec2& pos, const Vec2& size, const Vec4& color) {

	float clipPosX[4]{ clipPos.x, clipPos.x + borders.w, clipPos.x + clipSize.x - borders.y, clipPos.x + clipSize.x };
	float clipPosY[4]{ clipPos.y, clipPos.y + borders.x, clipPos.y + clipSize.y - borders.z, clipPos.y + clipSize.y };
	float posX[4]{ pos.x, pos.x + borders.w, pos.x + size.x - borders.y, pos.x + size.x };
	float posY[4]{ pos.y, pos.y + borders.x, pos.y + size.y - borders.z, pos.y + size.y };
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			drawTextureClip(
				texture,
				Vec2(clipPosX[x], clipPosY[y]),
				Vec2(clipPosX[x + 1], clipPosY[y + 1]) - Vec2(clipPosX[x], clipPosY[y]),
				Vec2(posX[x], posY[y]),
				Vec2(posX[x + 1], posY[y + 1]) - Vec2(posX[x], posY[y]),
				color
			);
		}
	}
}

void Gfx::drawText(Texture* font, const char* text, const Vec2& pos, const Vec4& color) {
	unsigned char ch;
	Vec2 offset(0, 0);
	while (ch = *text++) {
		if (ch == '\n') {
			offset.x = 0;
			offset.y += 8;
			continue;
		}

		int x = ((ch - 32) % 32) * 8;
		int y = ((ch - 32) / 32) * 8;
		drawTextureClip(font, Vec2(x, y), Vec2(8, 8), pos + offset, Vec2(8, 8), color);
		offset.x += 8;
	}
}

void Gfx::drawSprite(const Sprite& sprite, const Vec2& position, const Vec2& size, const Vec4& color, bool mirrored) {
	if (sprite.sliced) {
		drawTextureSliced(sprite.texture, sprite.clipPosition, sprite.clipSize, sprite.borders, position, size, color);
	}
	else {
		drawTextureClip(sprite.texture, sprite.clipPosition, sprite.clipSize, position + sprite.offset, size, color, mirrored);
	}
}

void Gfx::drawSprite(const Sprite& sprite, const Vec2& position, const Vec4& color, bool mirrored) {
	drawSprite(sprite, position, sprite.clipSize, color, mirrored);
}

void Gfx::drawRotatedSprite(const Sprite& sprite, const Vec2& position, float angle, const Vec4& color, bool mirrored) {
	if (currentSpriteTexture != sprite.texture) beginSprites(sprite.texture);
	auto uv = sprite.clipPosition / Vec2(sprite.texture->width(), sprite.texture->height());
	uv.y = 1 - uv.y;
	auto du = sprite.clipSize.x / sprite.texture->width();
	auto dv = -sprite.clipSize.y / sprite.texture->height();
	if (mirrored) {
		uv.x += du;
		du *= -1;
	}
	const float w = sprite.clipSize.x * pixelScale / 2;
	const float h = sprite.clipSize.y * pixelScale / 2;
	auto dx = Vec2(cos(angle), sin(angle)) * w;
	auto dy = Vec2(-sin(angle), cos(angle)) * h;
	spriteVertices.push_back({ position * pixelScale - dx - dy, uv, color });
	spriteVertices.push_back({ position * pixelScale + dx - dy, uv + Vec2(du, 0), color });
	spriteVertices.push_back({ position * pixelScale + dx + dy, uv + Vec2(du, dv), color });
	spriteVertices.push_back({ position * pixelScale - dx + dy, uv + Vec2(0, dv), color });
}


void Gfx::drawRadialProgressIndicator(const Vec2& position, const Vec2& size, float progress, const Vec4& color) {
	endSprites();
	spriteVertices.clear();

	const float w = size.x * pixelScale;
	const float h = size.y * pixelScale;
	spriteVertices.push_back({ position * pixelScale, {0, 1}, color });
	spriteVertices.push_back({ position * pixelScale + Vec2(w, 0), {1, 1}, color });
	spriteVertices.push_back({ position * pixelScale + Vec2(w, h), {1, 0}, color });
	spriteVertices.push_back({ position * pixelScale + Vec2(0, h), {0, 0}, color });

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	radialProgressShader->use();
	radialProgressShader->uniform("screenSize", Vec2(width_, height_));
	radialProgressShader->uniform("progress", progress);
	spriteMesh->setVertices(spriteVertices.data(), sizeof(SpriteVertex), spriteVertices.size());
	spriteMesh->bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

	spriteVertices.clear();
}