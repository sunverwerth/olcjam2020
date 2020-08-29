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

#include "Game.h"
#include "Gfx.h"
#include "Timer.h"
#include "Sprite.h"
#include <SDL2/SDL.h>
#include <cmath>

int countNewlines(const char* text) {
	int nl = 0;
	char ch;
	while (ch = *text++) {
		if (ch == '\n') nl++;
	}
	return nl;
}

int longestLine(const char* text) {
	int longest = 0;
	int current = 0;
	char ch;
	while (ch = *text++) {
		if (ch == '\n') current = 0;
		current++;
		if (current > longest) longest = current;
	}
	return longest;
}

Sprite sprite_box;
Sprite sprite_box_rough;
Sprite sprite_bubble;
Sprite sprite_bubble_tip;

void Game::start() {
	font = gfx.getTexture("media/textures/font.png");
	sprites = gfx.getTexture("media/textures/sprites.png");
	gfx.setPixelScale(2);

	sprite_box = { sprites, Vec2(0,0), Vec2(32,32) };
	sprite_box_rough = { sprites, Vec2(0,32), Vec2(32,32) };
	sprite_bubble = { sprites, Vec2(32,0), Vec2(32,32), true, Vec4(8, 8, 8, 8) };
	sprite_bubble_tip = { sprites, Vec2(64,0), Vec2(8,4) };
}

void Game::handleEvent(const SDL_Event& event) {
	if (event.type == SDL_QUIT) {
		keepRunning = false;
		return;
	}
}

void Game::prepareFrame() {
	auto t = timer.elapsedTime();
	for (int y = 0; y < 25; y++) {
		for (int x = 0; x < 20; x++) {
			Sprite& sprite = (y % 3) ? sprite_box : sprite_box_rough;
			gfx.drawSprite(sprite, Vec2(x * 32, y * 32), Vec2(32, 32));
		}
	}

	bubble("Hello there!", Vec2(100, 100), Vec2(0, 0));
}

void Game::bubble(const char* text, const Vec2& pos, const Vec2& tippos) {
	int len = strlen(text);
	float width = longestLine(text) * 8 + 8;
	float height = 16 + countNewlines(text) * 8;
	gfx.drawSprite(sprite_bubble, pos, Vec2(width, height));

	Vec2 tip = tippos;
	tip.y = pos.y + height - 1;
	if (tip.x < pos.x + 4) tip.x = pos.x + 4;
	if (tip.x > pos.x + width - 12) tip.x = pos.x + width - 12;
	gfx.drawSprite(sprite_bubble_tip, tip, Vec2(8, 4));

	gfx.drawText(font, text, pos + Vec2(4, 4), Vec4(0.7, 0.7, 0.7, 1));
	
	std::string fps = std::to_string(timer.fps()) + " fps";
	gfx.drawText(font, fps.c_str(), Vec2(0, 0));
}