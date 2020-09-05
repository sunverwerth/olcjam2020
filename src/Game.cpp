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
#include "AudioClip.h"
#include "Unit.h"
#include "utils.h"
#include "Level.h"
#include "Soldier.h"
#include "Rocket.h"
#include "Explosion.h"
#include "Drone.h"

#include <SDL2/SDL.h>
#include <cmath>
#include <sstream>

Vec2 mainCPUPosition;

Sprite sprite_bubble;
Sprite sprite_bubble_tip;
Sprite sprite_button;
Sprite sprite_button_pressed;

Sprite tiles[256];
int numTiles;

Sprite structures[256];

enum Structure {
	STRUCTURE_WALL = 0,
	STRUCTURE_HOUSE,
	STRUCTURE_ROAD_CROSS,
	STRUCTURE_ROAD_HORIZ,
	STRUCTURE_ROAD_VERT,
	STRUCTURE_ROAD_TOP_LEFT,
	STRUCTURE_ROAD_TOP_RIGHT,
	STRUCTURE_ROAD_BOTTOM_RIGHT,
	STRUCTURE_ROAD_BOTTOM_LEFT,
	STRUCTURE_ROAD_T_LEFT,
	STRUCTURE_ROAD_T_TOP,
	STRUCTURE_ROAD_T_RIGHT,
	STRUCTURE_ROAD_T_BOTTOM,
	STRUCTURE_CRATER,
	STRUCTURE_COMPUTE_CORE,
	STRUCTURE_SILICON_REFINERY,
	STRUCTURE_DRONE_DEPLOYER,
	STRUCTURE_COUNT,
};

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

struct BuildInfo {
	bool canBuildMultiple;
	float opsToBuild;
	const Sprite& sprite;
	float buildOpsRemaining{ 0 };
	int inProgressCount{ 0 };
	int readyCount{ 0 };

	void build() {
		if (inProgressCount <= 0 || canBuildMultiple) {
			if (inProgressCount == 0) buildOpsRemaining = opsToBuild;
			inProgressCount++;
		}
	}
};

BuildInfo wallBuildInfo{ true, 2, structures[STRUCTURE_WALL] };
BuildInfo floorBuildInfo{ true, 0.5, tiles[4] };
BuildInfo computeBuildInfo{ false, 10, structures[STRUCTURE_COMPUTE_CORE] };
BuildInfo siliconBuildInfo{ false, 20, structures[STRUCTURE_SILICON_REFINERY] };
BuildInfo droneBuildInfo{ false, 30, structures[STRUCTURE_DRONE_DEPLOYER] };

BuildInfo* buildInfos[]{ &wallBuildInfo, &floorBuildInfo, &computeBuildInfo, &siliconBuildInfo, &droneBuildInfo };


const int WAVE_DURATION = 30;
const int WAVE_SPACING = 6;

Level level(100, 100);
Vec2 cameraPosition{ 500,500 };
Vec2 cameraSpeed;

bool moveUp, moveDown, moveLeft, moveRight;

void Game::start() {
	srand(SDL_GetTicks());
	auto clip = sfx.getAudioClip("media/sounds/wind_loop.wav");
	wind_sound = sfx.loop(clip, 0.5, 0, 0.6);
	guitexture = gfx.getTexture("media/textures/gui.png");
	sprites = gfx.getTexture("media/textures/sprites.png");
	gfx.setPixelScale(2);
	gfx.setClearColor(Vec4::BLACK);

	for (int i = 0; i < 32; i++) {
		tiles[numTiles++] = { sprites, Vec2((i % 32) * 32, (i / 32) * 32), Vec2(32, 32) };
	}

	sprite_bubble = { guitexture, Vec2(0, 224), Vec2(32, 32), true, Vec4(8, 8, 8, 8) };
	sprite_bubble_tip = { guitexture, Vec2(32, 224), Vec2(8, 4) };
	sprite_button = { guitexture, Vec2(0, 206), Vec2(32, 18), true, Vec4(6, 6, 6, 6) };
	sprite_button_pressed = { guitexture, Vec2(32, 206), Vec2(32, 18), true, Vec4(6, 6, 6, 6) };

	for (int i = 0; i < STRUCTURE_COUNT; i++) {
		structures[i] = { sprites, Vec2(i * 32, 960), Vec2(32,64) };
	}

	Soldier::sprites[0] = { sprites, Vec2(0, 475), Vec2(6, 5) };
	Soldier::sprites[1] = { sprites, Vec2(0, 480), Vec2(6, 5) };
	Soldier::sprites[2] = { sprites, Vec2(0, 485), Vec2(6, 5) };
	Soldier::sprites[3] = { sprites, Vec2(0, 490), Vec2(6, 5) };
	Soldier::sprites[4] = { sprites, Vec2(0, 495), Vec2(6, 5) };
	Soldier::sprites[5] = { sprites, Vec2(0, 500), Vec2(6, 5) };

	Rocket::sprites[0] = { sprites, Vec2(0,508),Vec2(8,4) };
	Rocket::sprites[1] = { sprites, Vec2(0,512),Vec2(8,4) };
	Rocket::sprites[2] = { sprites, Vec2(8,508),Vec2(4,8) };
	Rocket::sprites[3] = { sprites, Vec2(12,508),Vec2(4,8) };

	Drone::sprites[0] = { sprites, Vec2(0,516),Vec2(8,8) };
	Drone::sprites[1] = { sprites, Vec2(8,516),Vec2(8,8) };

	Explosion::sprites[0] = { sprites, Vec2(0,524),Vec2(32,32) };
	Explosion::sprites[1] = { sprites, Vec2(32,524),Vec2(32,32) };
	Explosion::sprites[2] = { sprites, Vec2(64,524),Vec2(32,32) };

	sprite_dust = { sprites, Vec2(500,500), Vec2(1,1) };
	for (int i = 0; i < dustParticleCount; i++) {
		createParticle(dustParticles[i]);
	}

	level.load();

	for (int y = 0; y < level.height(); y++) {
		for (int x = 0; x < level.width(); x++) {
			const auto& str = level.getStructure(x, y);
			if (str == STRUCTURE_COMPUTE_CORE) {
				computingPower += 1;
				mainCPUPosition = Vec2(x * 32 + 16, y * 32 + 16);
			}
		}
	}

	nextWaveTime = timer.elapsedTime() + WAVE_SPACING;

	spawnDrone({ 700,100 });
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
		case SDLK_r: sprites->load(Image("media/textures/sprites.png")); return;
		case SDLK_F5: level.save(); return;
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
}

void Game::createParticle(DustParticle& p) {
	p.pos = cameraPosition + Vec2(rand() % int(gfx.width() / gfx.getPixelScale()), rand() % int(gfx.height() / gfx.getPixelScale()));
	p.speed = 0.5f + (float)rand() / RAND_MAX / 2;
	p.time = 0;
	p.color = Vec4(1, 0.9, 0.7, 1) * frand(0, 1);
	p.color.w = 1;
}

bool Game::inViewport(const Vec2& pos) const {
	float w = gfx.width() / gfx.getPixelScale();
	float h = gfx.width() / gfx.getPixelScale();
	if (pos.x < cameraPosition.x) return false;
	if (pos.y < cameraPosition.y) return false;
	if (pos.x > cameraPosition.x + w) return false;
	if (pos.y > cameraPosition.y + h) return false;
	return true;
}

void Game::spawnDrone(const Vec2& pos) {
	auto drone = new Drone(pos);
	units.push_back(drone);
}

void Game::spawnRocket(const Vec2& pos, const Vec2& target, float speed) {
	float pan = clamp((pos.x - gfx.width() / gfx.getPixelScale() - cameraPosition.x) / gfx.width(), -0.5, 0.5);
	sfx.play(sfx.getAudioClip("media/sounds/rocket.wav"), 0.1f, pan, frand(0.9, 1.1));

	auto rocket = new Rocket(pos, target, speed);
	units.push_back(rocket);
}

void Game::spawnExplosion(const Vec2& pos) {
	auto rpos = round(pos / 32);
	if (level.getStructure(rpos.x, rpos.y) == -1) {
		level.setStructure(rpos.x, rpos.y, 13);
	}

	for (auto& unit : units) {
		if (unit->inRadius(pos, 16)) {
			unit->takeExplosionDamage();
		}
	}

	float pan = clamp((pos.x - gfx.width() / gfx.getPixelScale() - cameraPosition.x) / gfx.width(), -0.5, 0.5);
	sfx.play(sfx.getAudioClip("media/sounds/explosion.wav"), 0.5f, pan, frand(0.5, 1.0));

	auto explosion = new Explosion(pos);
	units.push_back(explosion);
}

void Game::spawnSoldier() {
	auto soldier = new Soldier(mainCPUPosition + Vec2(frand(-1, 1), frand(-1, 1)).normalized() * 300);
	units.push_back(soldier);
}

void Game::prepareFrame() {
	tooltip = nullptr;
	unsigned int mouseState = SDL_GetMouseState(&mouseX, &mouseY);
	mousePressed = ~mouseButtons & mouseState;
	mouseReleased = mouseButtons & ~mouseState;
	mouseButtons = mouseState;

	auto t = timer.elapsedTime();
	float dt = timer.deltaTime();
	if (dt > 0.1f) dt = 0.1f;

	const int numRounds = 10;
	float gflops = computingPower * dt;
	float slice = gflops / numRounds;
	while (gflops > 0) {
		float substractGFlops = 0;
		for (auto& info : buildInfos) {
			if (info->inProgressCount <= 0) continue;
			info->buildOpsRemaining -= slice;
			substractGFlops += slice;
			if (info->buildOpsRemaining <= 0) {
				info->readyCount++;
				info->inProgressCount--;
				if (info->inProgressCount > 0) info->buildOpsRemaining = info->opsToBuild;
			}
		}
		if (substractGFlops <= 0) break;
		gflops -= substractGFlops;
	}

	if (timer.elapsedTime() >= nextWaveTime) {
		startWave();
	}

	if (timer.elapsedTime() < waveEnd) {
		doWave();
	}

	// units
	auto workUnits = units;
	for (auto& unit : workUnits) {
		unit->update(dt, *this, sfx);
	}
	for (auto& unit : units) {
		if (!unit->isAlive()) {
			delete unit;
			unit = nullptr;
		}
	}
	units.erase(std::remove(units.begin(), units.end(), nullptr), units.end());

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

	// render units
	auto drawunits = units;
	for (auto& unit : drawunits) {
		unit->draw(gfx);
	}

	// Render dust
	windSpeed = 300 + sin(t * 0.05) * cos(t * 0.051) * cos(t * 0.0511) * 100;
	windAngle += frand(-0.02f, 0.02f);

	auto windVector = Vec2(cos(windAngle), sin(windAngle));

	wind_sound->setVolume(windSpeed / 500);
	wind_sound->setPitch(windSpeed / 400);
	wind_sound->setPan(-windVector.x * 0.25f);

	windVector *= windSpeed;

	for (int i = 0; i < dustParticleCount; i++) {
		auto& p = dustParticles[i];
		p.time += dt;
		p.pos += windVector * p.speed * dt;
		if (p.time < 0) continue;
		if (p.time > 1 || !inViewport(p.pos)) {
			createParticle(p);
			continue;
		}
		Vec4 color = p.color;
		color.w = p.time > 0.75f ? 1.0f - (p.time - 0.75f) * 4 : 1;
		gfx.drawSprite(sprite_dust, p.pos - round(cameraPosition), color);
	}

	// Hud
	std::stringstream sstr;
	sstr << silicon << " Silicon | " << computingPower << " GFlops";
	if (nextWaveTime - timer.elapsedTime() <= 10) {
		sstr << " Wave " << (nextWaveLevel + 1) << " in " << int(nextWaveTime - timer.elapsedTime());
	}
	gfx.drawText(guitexture, sstr.str().c_str(), Vec2(1, 1), Vec4(0, 0, 0, 0.5));
	gfx.drawText(guitexture, sstr.str().c_str(), Vec2(0, 0));

	// GUI
	controlId = 0;

	prepareGUI();

	if (!(mouseButtons & SDL_BUTTON(1))) {
		activeControlId = 0;
	}

	if (tooltip) {
		float width = strlen(tooltip) * 8 + 8;
		auto bubblepos = Vec2(mouseX - 48, mouseY - 48) / gfx.getPixelScale();
		if (bubblepos.x + width > gfx.width() / gfx.getPixelScale()) {
			bubblepos.x -= bubblepos.x + width - gfx.width() / gfx.getPixelScale();
		}
		bubble(tooltip, bubblepos, Vec2(mouseX, mouseY) / gfx.getPixelScale());
	}
}

void Game::startWave() {
	nextWaveLevel++;
	waveEnd = timer.elapsedTime() + WAVE_DURATION;
	nextWaveTime = timer.elapsedTime() + WAVE_SPACING;
}

void Game::doWave() {
	if (nextWaveLevel == 1) {
		static double nextSoldier = 0;
		if (timer.elapsedTime() > nextSoldier) {
			nextSoldier = timer.elapsedTime() + 0.5;
			spawnSoldier();
		}
	}
}
void Game::buildButton(BuildInfo& info, const char* text, const Vec2& pos, const Vec2& size) {
	Vec2 windowPos = Vec2(gfx.width() / gfx.getPixelScale() - 80, 0);
	if (button(info.sprite, text,  pos, size)) {
		info.build();
	}
	if (info.buildOpsRemaining > 0) {
		gfx.drawRadialProgressIndicator(pos, size, info.buildOpsRemaining / info.opsToBuild, Vec4(0, 1, 0, 0.25));
	}
	if (info.inProgressCount > 0) {
		auto str = std::to_string(info.inProgressCount);
		gfx.drawText(guitexture, str.c_str(), pos + Vec2(3, 3), Vec4(0, 0, 0, 0.5));
		gfx.drawText(guitexture, str.c_str(), pos + Vec2(2, 2), Vec4::WHITE);
	}
	if (info.readyCount > 0) {
		auto str = std::to_string(info.readyCount);
		gfx.drawText(guitexture, str.c_str(), pos + size - Vec2(10, 10), Vec4(0, 0, 0, 0.5));
		gfx.drawText(guitexture, str.c_str(), pos + size - Vec2(11, 11), Vec4::GREEN);
	}
}

void Game::prepareGUI() {
	Vec2 windowPos = Vec2(gfx.width() / gfx.getPixelScale() - 80, 0);
	window("TGM v1.0", windowPos, Vec2(80, gfx.height() / gfx.getPixelScale()));
	
	buildButton(wallBuildInfo, "Build Wall", windowPos + Vec2(4, 32), Vec2(36, 68));
	buildButton(computeBuildInfo, "Build Compute Core", windowPos + Vec2(40, 32), Vec2(36, 68));
	buildButton(siliconBuildInfo, "Build Silicon Refinery", windowPos + Vec2(4, 100), Vec2(36, 68));
	buildButton(droneBuildInfo, "Build Drone Deployer", windowPos + Vec2(40, 100), Vec2(36, 68));
	buildButton(floorBuildInfo, "Build Floor", windowPos + Vec2(4, 168), Vec2(36, 36));

	if (button("Exit", Vec2(gfx.width() / gfx.getPixelScale() - 76, gfx.height() / gfx.getPixelScale() - 20), Vec2(72, 16))) {
		keepRunning = false;
	}
}

bool Game::isMouseOver(const Vec2& pos, const Vec2& size) {
	float scale = gfx.getPixelScale();
	if (mouseX / scale < pos.x) return false;
	if (mouseY / scale < pos.y) return false;
	if (mouseX / scale > pos.x + size.x) return false;
	if (mouseY / scale > pos.y + size.y) return false;
	return true;
}

void Game::bubble(const char* text, const Vec2& pos, const Vec2& tippos) {
	int len = strlen(text);
	float width = longestLine(text) * 8 + 8;
	float height = 16 + countNewlines(text) * 8;
	gfx.drawSprite(sprite_bubble, pos, Vec2(width, height));

	Vec2 tip = tippos;
	tip.x -= 4;
	tip.y = pos.y + height - 1;
	if (tip.x < pos.x + 4) tip.x = pos.x + 4;
	if (tip.x > pos.x + width - 12) tip.x = pos.x + width - 12;
	gfx.drawSprite(sprite_bubble_tip, tip);

	gfx.drawText(guitexture, text, pos + Vec2(4, 4), Vec4(0.7, 0.7, 0.7, 1));
}

bool Game::button(const char* text, const Vec2& pos, const Vec2& size) {
	controlId++;

	Vec2 realsize(size);
	float textwidth = strlen(text) * 8;
	if (realsize.x == 0) realsize.x = textwidth + 8;
	if (realsize.y == 0) realsize.y = 16;

	bool hover = isMouseOver(pos, realsize);
	if (hover && mousePressed & SDL_BUTTON(1)) activeControlId = controlId;
	bool press = mouseButtons & SDL_BUTTON(1);
	gfx.drawSprite(hover && press && activeControlId == controlId ? sprite_button_pressed : sprite_button, pos, realsize);
	gfx.drawText(guitexture, text, pos + realsize / 2 - Vec2(textwidth / 2, hover && press && activeControlId == controlId ? 3 : 4));

	if (hover && mouseReleased & SDL_BUTTON(1) && activeControlId == controlId) return true;
	return false;
}

bool Game::button(const Sprite& sprite, const char* buttonTooltip, const Vec2& pos, const Vec2& size) {
	controlId++;

	Vec2 realsize(size);
	if (realsize.x == 0) realsize.x = sprite.clipSize.x + 4;
	if (realsize.y == 0) realsize.y = sprite.clipSize.y + 4;

	bool hover = isMouseOver(pos, realsize);
	if (hover && mousePressed & SDL_BUTTON(1)) activeControlId = controlId;
	bool press = mouseButtons & SDL_BUTTON(1);
	gfx.drawSprite(hover && press && activeControlId == controlId ? sprite_button_pressed : sprite_button, pos, realsize);
	gfx.drawSprite(sprite, pos + realsize / 2 - sprite.clipSize / 2 + Vec2(0, hover && press && activeControlId == controlId ? 1 : 0));

	if (hover && mouseReleased & SDL_BUTTON(1) && activeControlId == controlId) return true;

	if (hover && buttonTooltip && *buttonTooltip) {
		tooltip = buttonTooltip;
	}

	return false;
}

void Game::window(const char* title, const Vec2& pos, const Vec2& size) {
	controlId++;

	gfx.drawSprite(sprite_button, pos, size);
	gfx.drawText(guitexture, title, pos + Vec2(size.x / 2 - strlen(title) * 4, 5));
}