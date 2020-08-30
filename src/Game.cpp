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
#include "Image.h"
#include "Texture.h"
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

Sprite sprite_bubble;
Sprite sprite_bubble_tip;

Sprite tiles[256];
int numTiles;
int selectedTile;

Sprite structures[256];
int numStructures;
int selectedStructure;

class Level {
public:
	Level(int width, int height);
	~Level();

	int width() const { return width_; }
	int height() const { return height_; }

	int getTile(int x, int y) const;
	void setTile(int x, int y, int tile);
	int getStructure(int x, int y) const;
	void setStructure(int x, int y, int structure);

private:
	void load();
	void save() const;

private:
	int width_;
	int height_;
	int* tiles;
	int* structures;
};

#include <fstream>
#include "sys.h"

Level::Level(int width, int height) : width_(width), height_(height), tiles(new int[width * height]), structures(new int[width * height]) {
	memset(tiles, 0, sizeof(int) * width * height);
	memset(structures, -1, sizeof(int) * width * height);
	load();
}

Level::~Level() {
	delete[] tiles;
	delete[] structures;
}

int Level::getTile(int x, int y) const {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return 0;
	return tiles[y * width_ + x];
}

void Level::setTile(int x, int y, int tile) {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

	tiles[y * width_ + x] = tile;
	save();
}

int Level::getStructure(int x, int y) const {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return 0;
	return structures[y * width_ + x];
}

void Level::setStructure(int x, int y, int tile) {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

	structures[y * width_ + x] = tile;
	save();
}

void Level::load() {
	std::ifstream file("media/level.dat");
	if (!file.good()) {
		log_error("Could not open level file for reading.");
		return;
	}
	file.read(reinterpret_cast<char*>(tiles), sizeof(int) * width_ * height_);
	file.read(reinterpret_cast<char*>(structures), sizeof(int) * width_ * height_);
}

void Level::save() const {
	std::ofstream file("media/level.dat");
	if (!file.good()) {
		log_error("Could not open level file for writing.");
		return;
	}
	file.write(reinterpret_cast<const char*>(tiles), sizeof(int) * width_ * height_);
	file.write(reinterpret_cast<const char*>(structures), sizeof(int) * width_ * height_);
}

Level level(100, 100);
Vec2 cameraPosition{ 500,500 };
Vec2 cameraSpeed;

bool moveUp, moveDown, moveLeft, moveRight;
enum class Mode {
	GAME,
	TILES,
	STRUCTURES,
} mode;

void Game::start() {
	guitexture = gfx.getTexture("media/textures/font.png");
	sprites = gfx.getTexture("media/textures/sprites.png");
	gfx.setPixelScale(2);

	for (int i = 0; i < 32; i++) {
		tiles[numTiles++] = { sprites, Vec2((i % 32) * 32, (i / 32) * 32), Vec2(32, 32) };
	}

	sprite_bubble = { guitexture, Vec2(0, 224), Vec2(32,32), true, Vec4(8, 8, 8, 8) };
	sprite_bubble_tip = { guitexture, Vec2(32, 224), Vec2(8,4) };

	structures[numStructures++] = { sprites, Vec2(0, 960), Vec2(32,64) };
}

void nextmode() {
	switch (mode) {
	case Mode::GAME: mode = Mode::TILES; break;
	case Mode::TILES: mode = Mode::STRUCTURES; break;
	case Mode::STRUCTURES: mode = Mode::GAME; break;
	}
}

void Game::handleEvent(const SDL_Event& event) {
	if (event.type == SDL_QUIT) {
		keepRunning = false;
		return;
	}

	switch (event.type) {
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_a: moveLeft = true; return;
		case SDLK_d: moveRight = true; return;
		case SDLK_w: moveUp = true; return;
		case SDLK_s: moveDown = true; return;
		case SDLK_TAB: nextmode(); return;
		case SDLK_PLUS: if (mode == Mode::TILES) selectedTile = (selectedTile + 1) % numTiles; return;
		case SDLK_MINUS: if (mode == Mode::TILES) selectedTile = (selectedTile + numTiles - 1) % numTiles; return;
		case SDLK_r: sprites->load(Image("media/textures/sprites.png")); return;
		}
		return;
	case SDL_KEYUP:
		switch (event.key.keysym.sym) {
		case SDLK_a: moveLeft = false; return;
		case SDLK_d: moveRight = false; return;
		case SDLK_w: moveUp = false; return;
		case SDLK_s: moveDown = false; return;
		}
		return;
	}
}

void Game::prepareFrame() {
	auto t = timer.elapsedTime();
	float dt = timer.deltaTime();

	// Movement
	if (moveLeft) cameraSpeed.x -= dt * 3000;
	if (moveRight) cameraSpeed.x += dt * 3000;
	if (moveUp) cameraSpeed.y -= dt * 3000;
	if (moveDown) cameraSpeed.y += dt * 3000;

	cameraPosition += cameraSpeed * dt;

	cameraSpeed *= pow(0.5f, dt * 15);

	// Find view rect
	int minx = (cameraPosition.x) / 32 - 1;
	int maxx = (cameraPosition.x + gfx.width() / gfx.getPixelScale() + 32) / 32;
	int miny = (cameraPosition.y) / 32 - 1;
	int maxy = (cameraPosition.y + gfx.height() / gfx.getPixelScale() + 32) / 32;

	// Render background
	for (int y = miny; y < maxy; y++) {
		for (int x = minx; x < maxx; x++) {
			Sprite& sprite = tiles[level.getTile(x, y)];
			gfx.drawSprite(sprite, Vec2(x * 32, y * 32) - round(cameraPosition));
		}
	}

	// Render structures
	for (int y = miny; y < maxy; y++) {
		for (int x = minx; x < maxx; x++) {
			int structure = level.getStructure(x, y);
			if (structure < 0) continue;
			Sprite& sprite = structures[structure];
			gfx.drawSprite(sprite, Vec2(x * 32, y * 32 - sprite.clipSize.y + 32) - round(cameraPosition));
		}
	}

	// Editor
	if (mode == Mode::TILES) {
		gfx.drawText(guitexture, "Place tiles", Vec2(300, 0), Vec4::BLACK);
		gfx.drawSprite(tiles[selectedTile], Vec2(0, 0));

		int mx, my;
		auto buttons = SDL_GetMouseState(&mx, &my);
		int x = (mx / gfx.getPixelScale() + cameraPosition.x) / 32;
		int y = (my / gfx.getPixelScale() + cameraPosition.y) / 32;
		gfx.drawSprite(tiles[selectedTile], Vec2(x * 32, y * 32) - round(cameraPosition));

		if (buttons & SDL_BUTTON(1)) {
			level.setTile(x, y, selectedTile);
		}
	}
	else if (mode == Mode::STRUCTURES) {
		gfx.drawText(guitexture, "Place structures", Vec2(300, 0), Vec4::BLACK);
		gfx.drawSprite(structures[selectedStructure], Vec2(0, 0));

		int mx, my;
		auto buttons = SDL_GetMouseState(&mx, &my);
		int x = (mx / gfx.getPixelScale() + cameraPosition.x) / 32;
		int y = (my / gfx.getPixelScale() + cameraPosition.y) / 32;
		const Sprite& structure = structures[selectedStructure];
		gfx.drawSprite(structure, Vec2(x * 32, y * 32 - structure.clipSize.y + 32) - round(cameraPosition));

		if (buttons & SDL_BUTTON(1)) {
			level.setStructure(x, y, selectedStructure);
		}
		if (buttons & SDL_BUTTON(3)) {
			level.setStructure(x, y, -1);
		}
	}

	// Hud
	std::string fps = std::to_string(timer.fps()) + " fps";
	gfx.drawText(guitexture, fps.c_str(), Vec2(0, 0));
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
	gfx.drawSprite(sprite_bubble_tip, tip);

	gfx.drawText(guitexture, text, pos + Vec2(4, 4), Vec4(0.7, 0.7, 0.7, 1));
}