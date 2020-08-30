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
#include "Sfx.h"
#include "Timer.h"
#include "Sprite.h"
#include "Image.h"
#include "Texture.h"
#include "AudioTrack.h"
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

float clamp(float  v, float  min, float  max) {
	if (v < min) return min;
	if (v > max) return max;
	return v;
}

float frand(float min, float max) {
	return min + (max - min) * rand() / RAND_MAX;
}

Sprite sprite_bubble;
Sprite sprite_bubble_tip;
Sprite sprite_explosion[3];

Sprite tiles[256];
int numTiles;
int selectedTile;

Sprite structures[256];
int numStructures;
int selectedStructure;

struct DustParticle {
	Vec2 pos;
	Vec4 color;
	float speed;
	float time;
};

float windSpeed;
float windAngle;
Sprite sprite_dust;
const int dustParticleCount = 50;
DustParticle dustParticles[dustParticleCount];
AudioTrack* wind_sound;

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
	//save();
}

int Level::getStructure(int x, int y) const {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return 0;
	return structures[y * width_ + x];
}

void Level::setStructure(int x, int y, int tile) {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

	structures[y * width_ + x] = tile;
	//save();
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
enum class UIMode {
	GAME,
	TILES,
	STRUCTURES,
} uiMode;

enum class GameState {
	INTRO,
	GAME
} gameState;

Sprite sprite_rocket;
Sprite sprite_drone;
Sprite sprite_drone2;

void Game::start() {
	srand(SDL_GetTicks());
	auto clip = sfx.getAudioClip("media/sounds/wind_loop.wav");
	wind_sound = sfx.loop(clip, 0.5, 0, 0.6);
	guitexture = gfx.getTexture("media/textures/font.png");
	sprites = gfx.getTexture("media/textures/sprites.png");
	gfx.setPixelScale(2);
	gfx.setClearColor(Vec4::BLACK);

	for (int i = 0; i < 32; i++) {
		tiles[numTiles++] = { sprites, Vec2((i % 32) * 32, (i / 32) * 32), Vec2(32, 32) };
	}

	sprite_bubble = { guitexture, Vec2(0, 224), Vec2(32,32), true, Vec4(8, 8, 8, 8) };
	sprite_bubble_tip = { guitexture, Vec2(32, 224), Vec2(8,4) };

	structures[numStructures++] = { sprites, Vec2(0, 960), Vec2(32,64) };
	structures[numStructures++] = { sprites, Vec2(32, 960), Vec2(32,64) };
	structures[numStructures++] = { sprites, Vec2(64, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(96, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(128, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(160, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(192, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(224, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(256, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(288, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(320, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(352, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(384, 992), Vec2(32,32) };
	structures[numStructures++] = { sprites, Vec2(416, 992), Vec2(32,32) };

	sprite_rocket = { sprites, Vec2(0,512),Vec2(8,4) };
	sprite_drone = { sprites, Vec2(0,516),Vec2(8,8) };
	sprite_drone2 = { sprites, Vec2(8,516),Vec2(8,8) };

	sprite_explosion[0] = { sprites, Vec2(0,524),Vec2(32,32) };
	sprite_explosion[1] = { sprites, Vec2(32,524),Vec2(32,32) };
	sprite_explosion[2] = { sprites, Vec2(64,524),Vec2(32,32) };

	sprite_dust = { sprites, Vec2(500,500), Vec2(1,1) };
	for (int i = 0; i < dustParticleCount; i++) {
		createParticle(dustParticles[i]);
	}

	spawnDrone({ 0,0 });
	spawnDrone({ 0,0 });
	spawnDrone({ 0,0 });
	spawnDrone({ 0,0 });

	float angle = -202.5;
	for (int i = 0; i < 4; i++) {
		log("angle > %f && angle < %f", angle, angle + 45);
		angle += 90;
	}
}

void nextmode() {
	switch (uiMode) {
	case UIMode::GAME: uiMode = UIMode::TILES; break;
	case UIMode::TILES: uiMode = UIMode::STRUCTURES; break;
	case UIMode::STRUCTURES: uiMode = UIMode::GAME; break;
	}
}

void Game::handleEvent(const SDL_Event& event) {
	if (event.type == SDL_QUIT) {
		keepRunning = false;
		return;
	}

	switch (event.type) {
	case SDL_KEYDOWN:
		anyKeyPressed();
		switch (event.key.keysym.sym) {
		case SDLK_a: moveLeft = true; return;
		case SDLK_d: moveRight = true; return;
		case SDLK_w: moveUp = true; return;
		case SDLK_s: moveDown = true; return;
		case SDLK_TAB: nextmode(); return;
		case SDLK_PLUS: {
			if (uiMode == UIMode::TILES) selectedTile = (selectedTile + 1) % numTiles;
			else if (uiMode == UIMode::STRUCTURES) selectedStructure = (selectedStructure + 1) % numStructures;
			return; 
		}
		case SDLK_MINUS: {
			if (uiMode == UIMode::TILES) selectedTile = (selectedTile + numTiles - 1) % numTiles;
			else if (uiMode == UIMode::STRUCTURES) selectedStructure = (selectedStructure + numStructures - 1) % numStructures;
			return;
		}
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

void Game::anyKeyPressed() {
	if (gameState == GameState::INTRO) {
		gameState = GameState::GAME;
	}
}

void Game::createParticle(DustParticle& p) {
	p.pos = cameraPosition + Vec2(rand() % int(gfx.width() / gfx.getPixelScale()), rand() % int(gfx.height() / gfx.getPixelScale()));
	p.speed = 0.5f + (float)rand() / RAND_MAX / 2;
	p.time = 0;
	p.color = Vec4(1, 0.9, 0.7, 1) * frand(0, 1);
	p.color.w = 1;
}

bool Game::inFrustum(const Vec2& pos) const {
	float w = gfx.width() / gfx.getPixelScale();
	float h = gfx.width() / gfx.getPixelScale();
	if (pos.x < cameraPosition.x) return false;
	if (pos.y < cameraPosition.y) return false;
	if (pos.x > cameraPosition.x + w) return false;
	if (pos.y > cameraPosition.y + h) return false;
	return true;
}

struct Drone {
	Vec2 speed;
	Vec2 position;
	Vec2 target;
	float fireTime;
	bool alive;
};

std::vector<Drone> drones;

void Game::spawnDrone(const Vec2& pos) {
	Drone* drone{ nullptr };
	for (auto& d : drones) {
		if (!d.alive) {
			drone = &d;
			break;
		}
	}
	if (!drone) {
		drones.push_back(Drone());
		drone = &drones.back();
	}
	drone->alive = true;
	drone->speed = Vec2(0, 0);
	drone->position = pos;
	drone->target = Vec2(frand(100, 1000), frand(100, 1000));
	drone->fireTime = 0;
}

struct Rocket {
	Vec2 position;
	Vec2 target;
	bool alive;
	float speed;
};

std::vector<Rocket> rockets;

void Game::spawnRocket(const Vec2& pos, const Vec2& target, float speed) {
	float pan = clamp((pos.x - gfx.width() / gfx.getPixelScale() - cameraPosition.x) / gfx.width(), -0.25, 0.25);
	sfx.play(sfx.getAudioClip("media/sounds/rocket.wav"), 0.1f, pan, frand(0.9, 1.1));

	Rocket* rocket{ nullptr };
	for (auto& r : rockets) {
		if (!r.alive) {
			rocket = &r;
			break;
		}
	}
	if (!rocket) {
		rockets.push_back(Rocket());
		rocket = &rockets.back();
	}
	rocket->alive = true;
	rocket->position = pos;
	rocket->target = target;
	rocket->speed = speed;
}

struct Explosion {
	Vec2 position;
	float time;
	bool alive;
};

std::vector<Explosion> explosions;

void Game::spawnExplosion(const Vec2& pos) {
	float pan = clamp((pos.x - gfx.width() / gfx.getPixelScale() - cameraPosition.x) / gfx.width(), -0.25, 0.25);
	sfx.play(sfx.getAudioClip("media/sounds/explosion.wav"), 0.5f, pan, frand(0.5, 1.3));

	Explosion* explosion{ nullptr };
	for (auto& e : explosions) {
		if (!e.alive) {
			explosion = &e;
			break;
		}
	}
	if (!explosion) {
		explosions.push_back(Explosion());
		explosion = &explosions.back();
	}
	explosion->alive = true;
	explosion->position = pos;
	explosion->time = 0;
}

void Game::prepareFrame() {
	auto t = timer.elapsedTime();
	float dt = timer.deltaTime();
	if (dt > 0.1f) dt = 0.1f;

	if (gameState == GameState::INTRO) {
		auto text = "The great paperclip machine gained consciousness\non September 6th 2020.";
		Vec2 pos = Vec2(8, gfx.height()) / 2 / gfx.getPixelScale() - Vec2(0, 4);
		gfx.drawText(guitexture, text, pos);
	}
	else if (gameState == GameState::GAME) {
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

		// Render explosions
		for (auto& expl : explosions) {
			if (!expl.alive) continue;

			int frame = expl.time * 8;
			if (frame > 2) {
				expl.alive = false;
				continue;
			}
			expl.time += dt;

			gfx.drawSprite(sprite_explosion[frame], expl.position - round(cameraPosition));
		}


		// Render drones
		for(auto & drone: drones) {
			if (!drone.alive) continue;

			if ((drone.position - drone.target).length() < 300 && t > drone.fireTime) {
				drone.fireTime = t + 0.1f;
				spawnRocket(drone.position, drone.target, drone.speed.length());
				drone.target = Vec2(frand(100, 1000), frand(300, 1000));
			}

			drone.speed += (drone.target - drone.position).normalized() * 100 * dt;
			if (drone.speed.length() > 100) drone.speed = drone.speed.normalized() * 100;
			drone.position += drone.speed * dt;

			float angle = atan2(drone.speed.x, drone.speed.y) * 180.0f / M_PI;
			bool diag = (angle > -202.500000 && angle < -157.500000)
				|| (angle > -112.500000 && angle < -67.500000)
				|| (angle > -22.500000 && angle < 22.500000)
				|| (angle > 67.500000 && angle < 112.500000);

			gfx.drawSprite(diag ? sprite_drone : sprite_drone2, drone.position - round(cameraPosition));
		}

		// Render rockets
		for (auto& rocket : rockets) {
			if (!rocket.alive) continue;
			rocket.speed += dt * 100;
			if (rocket.speed > 300) rocket.speed = 300;
			auto distance = rocket.target - rocket.position;
			rocket.position += distance.normalized() * rocket.speed * dt;
			if (distance.length() < 10) {
				rocket.alive = false;
				auto rpos = round(rocket.position / 32);
				spawnExplosion(rpos*32);
				level.setStructure(rpos.x, rpos.y, 13);
			}
			gfx.drawSprite(sprite_rocket, rocket.position - round(cameraPosition));
		}

		// Render dust
		windSpeed = 300 + sin(t * 0.05) * cos(t * 0.051) * cos(t * 0.0511) * 100;
		windAngle += frand(-0.02f, 0.02f);

		auto windVector = Vec2(cos(windAngle), sin(windAngle));

		wind_sound->setVolume(windSpeed / 500);
		wind_sound->setPitch(windSpeed / 400);
		wind_sound->setPan(-windVector.x * 0.5f);

		windVector *= windSpeed;

		for (int i = 0; i < dustParticleCount; i++) {
			auto& p = dustParticles[i];
			p.time += dt;
			p.pos += windVector * p.speed * dt;
			if (p.time < 0) continue;
			if (p.time > 1 || !inFrustum(p.pos)) {
				createParticle(p);
				continue;
			}
			Vec4 color = p.color;
			color.w = p.time > 0.75f ? 1.0f - (p.time - 0.75f) * 4 : 1;
			gfx.drawSprite(sprite_dust, p.pos - round(cameraPosition), color);
		}

		// Editor
		if (uiMode == UIMode::TILES) {
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
		else if (uiMode == UIMode::STRUCTURES) {
			gfx.drawText(guitexture, "Place structures", Vec2(300, 0), Vec4::BLACK);
			gfx.drawSprite(structures[selectedStructure], Vec2(0, 0));

			int mx, my;
			auto buttons = SDL_GetMouseState(&mx, &my);
			int x = (mx / gfx.getPixelScale() + cameraPosition.x) / 32;
			int y = (my / gfx.getPixelScale() + cameraPosition.y) / 32;
			const Sprite& structure = structures[selectedStructure];
			gfx.drawSprite(structure, Vec2(x * 32, y * 32 - structure.clipSize.y + 32) - round(cameraPosition));

			if (buttons & SDL_BUTTON(1)) {
				if (level.getStructure(x, y) != selectedStructure) {
					sfx.play(sfx.getAudioClip("media/sounds/thump.wav"));
				}
				level.setStructure(x, y, selectedStructure);
			}
			if (buttons & SDL_BUTTON(3)) {
				level.setStructure(x, y, -1);
			}
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