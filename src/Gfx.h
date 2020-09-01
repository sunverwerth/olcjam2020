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

#pragma once

#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Mesh.h"
#include "SpriteVertex.h"
#include <SDL2/SDL.h>
#include <vector>
#include <map>
#include <string>

class Texture;
class Shader;
struct Sprite;

class Gfx {
public:
	Gfx(const char* title, int width, int height, bool fullscreen);
	~Gfx();

	int width() const { return width_; }
	int height() const { return height_; }

	float getPixelScale() const { return pixelScale; }
	void setPixelScale(float scale) { pixelScale = scale; }
	void setClearColor(const Vec4& color) { clearColor = color; }

	void beginFrame();
	void endFrame();

	Texture* getTexture(const char* name);

	void bindTexture(Texture*);

	void drawTexture(Texture* texture, const Vec2& pos, const Vec4& color = Vec4::WHITE);
	void drawTextureClip(Texture* texture, const Vec2& clipPos, const Vec2& clipSize, const Vec2& pos, const Vec2& size, const Vec4& color = Vec4::WHITE, bool mirror = false);
	void drawTextureSliced(Texture* texture, const Vec2& clipPos, const Vec2& clipSize, const Vec4& borders, const Vec2& pos, const Vec2& size, const Vec4& color = Vec4::WHITE);
	void drawText(Texture* dont, const char* text, const Vec2& pos, const Vec4& color = Vec4::WHITE);
	void drawSprite(const Sprite&, const Vec2& position, const Vec2& size, const Vec4& color = Vec4::WHITE, bool mirror = false);
	void drawSprite(const Sprite&, const Vec2& position, const Vec4& color = Vec4::WHITE, bool mirror = false);
	void drawRadialProgressIndicator(const Vec2& position, const Vec2& size, float progress, const Vec4& color = Vec4::WHITE);

private:
	void beginSprites(Texture* texture);
	void endSprites();

private:
	SDL_Window* window{ nullptr };
	SDL_GLContext context{ nullptr };
	int width_;
	int height_;
	Vec4 clearColor{ 1, 0, 1, 1 };
	std::vector<Texture*> textureUnits;

	// Sprite stuff
	float pixelScale{ 1 };
	Shader* spriteShader{ nullptr };
	Shader* radialProgressShader{ nullptr };
	Texture* currentSpriteTexture{ nullptr };
	std::vector<SpriteVertex> spriteVertices;
	Mesh* spriteMesh{ nullptr };

	// Resources
	std::map<std::string, Texture*> loadedTextures;
};
