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
#include "ComputeCore.h"
#include "Wall.h"
#include "SiliconRefinery.h"
#include "DroneDeployer.h"
#include "Crater.h"
#include "Grenade.h"
#include "Jet.h"

#include <SDL2/SDL.h>
#include <cmath>
#include <sstream>

Level level(100, 100);
Vec2 cameraPosition{ 500,500 };
Vec2 cameraSpeed;

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
	STRUCTURE_REPAIR_DRONE_DEPLOYER,
	STRUCTURE_COUNT,
};

bool is_floor_structure[]{
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
};

struct DustParticle {
	Vec2 pos;
	Vec4 color;
	float speed;
	float time;
};

float windSpeed;
float windAngle;
Vec2 windVector;

Sprite sprite_dust;
const int dustParticleCount = 50;
DustParticle dustParticles[dustParticleCount];
AudioTrack* wind_sound;
const char* tooltip;
const char* messageText;
float messageTimer;

void message(const char* txt) {
	messageText = txt;
	messageTimer = 1;
}

struct BuildInfo {
	bool canBuildMultiple;
	float opsToBuild;
	float siliconToBuild;
	const Sprite& sprite;
	float buildOpsRemaining{ 0 };
	int inProgressCount{ 0 };
	int readyCount{ 0 };
	std::string tooltip;

	BuildInfo(const char* name, const char* desc, bool canBuildMultiple, float opsToBuild, float siliconToBuild, const Sprite& sprite)
		: canBuildMultiple(canBuildMultiple),
		opsToBuild(opsToBuild),
		siliconToBuild(siliconToBuild),
		sprite(sprite)
	{
		std::stringstream sstr;
		sstr << name << "\n";
		sstr << siliconToBuild << " Si\n";
		sstr << opsToBuild << " GFlops\n";
		sstr << desc;

		tooltip = sstr.str();
	}

	bool build() {
		if (inProgressCount <= 0 || canBuildMultiple) {
			if (inProgressCount == 0) buildOpsRemaining = opsToBuild;
			inProgressCount++;
			return true;
		}
		return false;
	}

	virtual void place(int x, int y, Game& game, Sfx& sfx) = 0;
	virtual bool canPlace(int x, int y, Game& game) = 0;
};

struct FloorBuildInfo : public BuildInfo {
	FloorBuildInfo(const char* name, const char* desc, bool canBuildMultiple, float opsToBuild, float siliconToBuild, const Sprite& sprite)
		: BuildInfo(name, desc, canBuildMultiple, opsToBuild, siliconToBuild, sprite) {}

	virtual void place(int x, int y, Game& game, Sfx& sfx) override {
		if (canPlace(x, y, game)) {
			level.setTile(x, y, 1 + rand() % 4);
			sfx.play(sfx.getAudioClip("media/sounds/thump.wav"));
			readyCount--;
		}
	}

	virtual bool canPlace(int x, int y, Game& game) override {
		if (level.getTile(x, y) != 0) {
			message("Can not place floor here");
			return false;
		}
		return true;
	}
};

template<typename T> struct StructureBuildInfo : public BuildInfo {
	StructureBuildInfo(const char* name, const char* desc, bool canBuildMultiple, float opsToBuild, float siliconToBuild, const Sprite& sprite, void(*fixup)(T*) = nullptr)
		: BuildInfo(name, desc, canBuildMultiple, opsToBuild, siliconToBuild, sprite), fixup(fixup) {}

	virtual void place(int x, int y, Game& game, Sfx& sfx) override {
		if (canPlace(x, y, game)) {
			sfx.play(sfx.getAudioClip("media/sounds/thump.wav"));
			readyCount--;
			auto pos = Vec2(x, y) * 32;
			auto instance = game.spawn<T>(pos);
			if (fixup) fixup(instance);
		}
	}

	virtual bool canPlace(int x, int y, Game& game) override {
		auto structure = level.getStructure(x, y);
		if (!level.getUnits(x, y).empty()) {
			message("This space is already occupied");
			return false;
		}

		auto tile = level.getTile(x, y);
		if (tile < 1 || tile > 4) {
			message("Structure must be placed on floor tile");
			return false;
		}

		return true;
	}

	void(*fixup)(T*) { nullptr };
};

StructureBuildInfo<Wall> wallBuildInfo("Wall", "Protection against infantry", true, 100, 5, structures[STRUCTURE_WALL]);
FloorBuildInfo floorBuildInfo("Floor", "To place buildings", true, 100, 5, tiles[4]);
StructureBuildInfo<ComputeCore> computeBuildInfo("Compute Core", "Generates 1337 GFlops per second", false, 50000, 1000, structures[STRUCTURE_COMPUTE_CORE]);
StructureBuildInfo<SiliconRefinery> siliconBuildInfo("Silicon Refinery", "Produces 30 Silicon per second", false, 30000, 500, structures[STRUCTURE_SILICON_REFINERY]);
StructureBuildInfo<DroneDeployer> droneBuildInfo("Attack Drone Deployer", "Deploys attack drones", false, 100000, 5000, structures[STRUCTURE_DRONE_DEPLOYER]);
StructureBuildInfo<DroneDeployer> repairDroneBuildInfo("Repair Drone Deployer", "Deploys repair drones", false, 200000, 5000, structures[STRUCTURE_REPAIR_DRONE_DEPLOYER], [](DroneDeployer* dd) {
	dd->repair = true;
	});

BuildInfo* buildInfos[]{ &wallBuildInfo, &floorBuildInfo, &computeBuildInfo, &siliconBuildInfo, &droneBuildInfo, &repairDroneBuildInfo };


const int WAVE_DURATION = 30;
const int WAVE_SPACING = 90;

void Game::addUnit(Unit* unit, const Vec2& pos) {
	if (unit->isComputeCore()) computingPower += 1337;
	if (unit->isSiliconRefinery()) siliconPerSecond += 30;

	auto rpos = floor(pos / 32);
	int x = rpos.x;
	int y = rpos.y;
	level.addUnit(x, y, unit);
}

void Game::removeUnit(Unit* unit, const Vec2& pos) {
	if (unit->isComputeCore()) computingPower -= 1337;
	if (unit->isSiliconRefinery()) siliconPerSecond -= 25;

	auto rpos = floor(pos / 32);
	int x = rpos.x;
	int y = rpos.y;
	level.removeUnit(x, y, unit);
}

void Game::moveUnit(Unit* unit, const Vec2& from, const Vec2& to) {
	auto rfrom = floor(from / 32);
	auto rto = floor(to / 32);
	if (rfrom.x == rto.x && rfrom.y == rto.y) return;
	removeUnit(unit, from);
	addUnit(unit, to);
}

bool moveUp, moveDown, moveLeft, moveRight;

void Game::start() {
	srand(SDL_GetTicks());
	auto clip = sfx.getAudioClip("media/sounds/wind_loop.wav");
	wind_sound = sfx.loop(clip, 0.5, 0, 0.6);
	guiTexture = gfx.getTexture("media/textures/gui.png");
	spriteTexture = gfx.getTexture("media/textures/sprites.png");
	gfx.setPixelScale(2);
	gfx.setClearColor(Vec4::BLACK);

	for (int i = 0; i < 32; i++) {
		tiles[numTiles++] = { spriteTexture, Vec2((i % 32) * 32, (i / 32) * 32), Vec2(32, 32) };
	}

	sprite_bubble = { guiTexture, Vec2(0, 224), Vec2(32, 32), Vec2(0, 0), true, Vec4(8, 8, 8, 8) };
	sprite_bubble_tip = { guiTexture, Vec2(32, 224), Vec2(8, 4) };
	sprite_button = { guiTexture, Vec2(0, 206), Vec2(32, 18), Vec2(0, 0), true, Vec4(6, 6, 6, 6) };
	sprite_button_pressed = { guiTexture, Vec2(32, 206), Vec2(32, 18), Vec2(0, 0), true, Vec4(6, 6, 6, 6) };

	for (int i = 0; i < STRUCTURE_COUNT; i++) {
		structures[i] = { spriteTexture, Vec2(i * 32, 960), Vec2(32,64) };
	}

	Soldier::sprites[0] = { spriteTexture, Vec2(0, 475), Vec2(6, 5) };
	Soldier::sprites[1] = { spriteTexture, Vec2(0, 480), Vec2(6, 5) };
	Soldier::sprites[2] = { spriteTexture, Vec2(0, 485), Vec2(6, 5) };
	Soldier::sprites[3] = { spriteTexture, Vec2(0, 490), Vec2(6, 5) };
	Soldier::sprites[4] = { spriteTexture, Vec2(0, 495), Vec2(6, 5) };
	Soldier::sprites[5] = { spriteTexture, Vec2(0, 500), Vec2(6, 5) };

	Rocket::sprite = { spriteTexture, Vec2(0,509),Vec2(4, 1) };

	Grenade::sprite = { spriteTexture, Vec2(16, 508),Vec2(2, 2) };

	Drone::sprite = { spriteTexture, Vec2(0,516),Vec2(8,8) };

	Crater::sprite = { spriteTexture, Vec2(416, 992), Vec2(32, 32) };

	Jet::sprites[0] = { spriteTexture, Vec2(0, 408), Vec2(34, 22) };
	Jet::sprites[1] = { spriteTexture, Vec2(0, 430), Vec2(34, 22) };

	Explosion::sprites[0] = { spriteTexture, Vec2(0,524),Vec2(32,32) };
	Explosion::sprites[1] = { spriteTexture, Vec2(32,524),Vec2(32,32) };
	Explosion::sprites[2] = { spriteTexture, Vec2(64,524),Vec2(32,32) };

	Wall::sprites[0] = { spriteTexture, Vec2(0, 896), Vec2(32, 64), Vec2(0, -32) };
	Wall::sprites[1] = { spriteTexture, Vec2(0, 960), Vec2(32, 64), Vec2(0, -32) };

	ComputeCore::sprites[0] = { spriteTexture, Vec2(448, 896), Vec2(32, 64), Vec2(0, -32) };
	ComputeCore::sprites[1] = { spriteTexture, Vec2(448, 960), Vec2(32, 64), Vec2(0, -32) };

	SiliconRefinery::sprites[0] = { spriteTexture, Vec2(480, 896), Vec2(32, 64), Vec2(0, -32) };
	SiliconRefinery::sprites[1] = { spriteTexture, Vec2(480, 960), Vec2(32, 64), Vec2(0, -32) };

	DroneDeployer::sprites[0] = { spriteTexture, Vec2(512, 896), Vec2(32, 64), Vec2(0, -32) };
	DroneDeployer::sprites[1] = { spriteTexture, Vec2(512, 960), Vec2(32, 64), Vec2(0, -32) };
	DroneDeployer::sprites[2] = { spriteTexture, Vec2(544, 896), Vec2(32, 64), Vec2(0, -32) };
	DroneDeployer::sprites[3] = { spriteTexture, Vec2(544, 960), Vec2(32, 64), Vec2(0, -32) };

	sprite_dust = { spriteTexture, Vec2(500,500), Vec2(1,1) };
	for (int i = 0; i < dustParticleCount; i++) {
		createParticle(dustParticles[i]);
	}

	level.load();

	for (int y = 0; y < level.height(); y++) {
		for (int x = 0; x < level.width(); x++) {
			const auto& str = level.getStructure(x, y);
			if (str == STRUCTURE_COMPUTE_CORE) {
				mainCPUPosition = Vec2(x * 32 + 16, y * 32 + 16);
			}
		}
	}

	nextWaveTime = timer.elapsedTime() + WAVE_SPACING;

	auto cpu = new ComputeCore(Vec2(800, 704));
	units.push_back(cpu);
	addUnit(cpu, cpu->pos);
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
		case SDLK_r: {
			spriteTexture->load(Image("media/textures/sprites.png"));
			guiTexture->load(Image("media/textures/gui.png"));
			return;
		}
				   //case SDLK_F5: level.save(); return;
		case SDLK_PLUS: nextWaveTime = timer.elapsedTime(); return;
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

Drone* Game::spawnDrone(const Vec2& pos, bool repair) {
	auto drone = new Drone(pos);
	drone->repair = repair;
	units.push_back(drone);
	addUnit(drone, pos);
	return drone;
}

void Game::spawnRocket(const Vec2& pos, const Vec2& target, float speed, Faction faction) {
	float pan = clamp((pos.x - gfx.width() / gfx.getPixelScale() - cameraPosition.x) / gfx.width(), -0.5, 0.5);
	sfx.play(sfx.getAudioClip("media/sounds/rocket.wav"), 0.1f, pan, frand(0.9, 1.1));

	auto rocket = new Rocket(pos, target, speed, faction);
	units.push_back(rocket);
	addUnit(rocket, pos);
}

void Game::spawnExplosion(const Vec2& pos, bool small, Faction faction) {
	auto rpos = floor(pos / 32);
	if (level.getStructure(rpos.x, rpos.y) == -1) {
		level.setStructure(rpos.x, rpos.y, 13);
	}

	for (auto& unit : units) {
		if (unit->inRadius(pos, 24)) {
			unit->damage(small ? damage_grenade : damage_explosion, faction);
		}
	}

	float pan = clamp((pos.x - gfx.width() / gfx.getPixelScale() - cameraPosition.x) / gfx.width(), -0.5, 0.5);
	sfx.play(sfx.getAudioClip("media/sounds/explosion.wav"), small ? 0.2f : 0.5f, pan, small ? frand(1.5, 2) : frand(0.5, 1.0));

	auto explosion = new Explosion(pos);
	units.push_back(explosion);
	addUnit(explosion, pos);

	for (auto unit : level.getUnits(rpos.x, rpos.y)) {
		if (unit->isCrater()) return;
	}
	auto crater = new Crater(rpos * 32);
	units.push_back(crater);
	addUnit(crater, crater->pos);
}

void Game::spawnSoldier() {
	auto soldier = new Soldier(mainCPUPosition + Vec2(frand(-1, 1), frand(-1, 1)).normalized() * 300);
	units.push_back(soldier);
	addUnit(soldier, soldier->pos);
	if (rand() % 50 < nextWaveLevel) soldier->grenadier = true;
}

Grenade* Game::spawnGrenade(const Vec2& pos, const Vec2& target, Faction faction) {
	auto grenade = new Grenade(pos, target, faction);
	units.push_back(grenade);
	addUnit(grenade, grenade->pos);
	return grenade;
}

void Game::update() {
	tooltip = nullptr;
	unsigned int mouseState = SDL_GetMouseState(&mouseX, &mouseY);
	mousePressed = ~mouseButtons & mouseState;
	mouseReleased = mouseButtons & ~mouseState;
	mouseButtons = mouseState;

	auto t = timer.elapsedTime();
	float dt = timer.deltaTime();
	if (dt > 0.1f) dt = 0.1f;

	messageTimer -= dt;
	if (splash < 1) {
		splash -= dt;
	}
	if (splash < 0) {
		splash = 0;
	}

	if (computingPower <= 0 && gameOver == 0) {
		gameOver = 0.01f;
	}
	if (gameOver > 0) {
		gameOver += dt;
	}
	if (gameOver > 1) {
		gameOver = 1;
	}

	// distribute gflops
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

	// refine silicon
	silicon += siliconPerSecond * dt;

	if (timer.elapsedTime() >= nextWaveTime) {
		startWave();
	}

	if (timer.elapsedTime() < waveEnd) {
		doWave();
	}

	// place objects
	if (mouseX < gfx.width() - 80 * gfx.getPixelScale() && selectedBuildInfo && selectedBuildInfo->readyCount > 0) {
		int x = (mouseX / gfx.getPixelScale() + cameraPosition.x) / 32;
		int y = (mouseY / gfx.getPixelScale() + cameraPosition.y) / 32;
		if (mousePressed & SDL_BUTTON(1)) {
			selectedBuildInfo->place(x, y, *this, sfx);
		}
	}

	// units
	auto workUnits = units;
	for (auto& unit : workUnits) {
		auto oldPos = unit->pos;
		unit->update(dt, *this, sfx);
		moveUnit(unit, oldPos, unit->pos);
	}
	for (auto& unit : units) {
		if (!unit->isAlive()) {
			removeUnit(unit, unit->pos);
			//delete unit;
			unit = nullptr;
		}
	}
	units.erase(std::remove(units.begin(), units.end(), nullptr), units.end());

	// update wind
	windSpeed = 300 + sin(t * 0.05) * cos(t * 0.051) * cos(t * 0.0511) * 100;
	windAngle += frand(-0.02f, 0.02f);
	windVector = Vec2(cos(windAngle), sin(windAngle));
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
	}

	// Movement
	if (moveLeft) cameraSpeed.x -= dt * 3000;
	if (moveRight) cameraSpeed.x += dt * 3000;
	if (moveUp) cameraSpeed.y -= dt * 3000;
	if (moveDown) cameraSpeed.y += dt * 3000;

	cameraPosition += cameraSpeed * dt;

	cameraSpeed *= pow(0.5f, dt * 15);
}

void Game::drawFrame() {
	// Find view rect
	int minx = (cameraPosition.x) / 32 - 1;
	int maxx = (cameraPosition.x + gfx.width() / gfx.getPixelScale() + 32) / 32;
	int miny = (cameraPosition.y) / 32 - 1;
	int maxy = (cameraPosition.y + gfx.height() / gfx.getPixelScale() + 32) / 32;

	// Render floor tiles and floor structures
	for (int y = miny; y < maxy; y++) {
		for (int x = minx; x < maxx; x++) {
			int structure = level.getStructure(x, y);
			auto& units = level.getUnits(x, y);

			// Floor tile
			Sprite& sprite = tiles[level.getTile(x, y)];
			gfx.drawSprite(sprite, Vec2(x * 32, y * 32) - floor(cameraPosition));

			// Floor Structure
			for (auto unit : units) {
				unit->draw_floor(gfx);
			}
		}
	}

	if (selectedBuildInfo && selectedBuildInfo->readyCount > 0) {
		auto pos = Vec2(mouseX, mouseY) / gfx.getPixelScale();
		gfx.drawSprite(selectedBuildInfo->sprite, pos - selectedBuildInfo->sprite.clipSize / 2);
	}

	// Render normal structures and units
	for (int y = miny; y < maxy; y++) {
		for (int x = minx; x < maxx; x++) {
			int structure = level.getStructure(x, y);
			auto& units = level.getUnits(x, y);

			// Unit bottom
			for (auto unit : units) {
				unit->draw_bottom(gfx);
			}

			// Normal structure
			for (auto unit : units) {
				unit->draw_structure(gfx);
			}

			// Unit top
			for (auto unit : units) {
				unit->draw_top(gfx);
			}
		}
	}

	if (splash > 0) {
		gfx.drawSprite(sprite_dust, Vec2(0, 0), Vec2(1000, 1000), Vec4(0, 0, 0, splash));
	}

	// Render dust
	for (int i = 0; i < dustParticleCount; i++) {
		auto& p = dustParticles[i];
		Vec4 color = p.color;
		color.w = p.time > 0.75f ? 1.0f - (p.time - 0.75f) * 4 : 1;
		gfx.drawSprite(sprite_dust, p.pos - floor(cameraPosition), color);
	}

	// Render GUI
	controlId = 0;

	prepareGUI();

	if (!(mouseButtons & SDL_BUTTON(1))) {
		activeControlId = 0;
	}

	if (tooltip) {
		float width = longestLine(tooltip) * 8 + 8;
		auto bubblepos = Vec2(mouseX - 48, mouseY - countNewlines(tooltip) * 16 - 48) / gfx.getPixelScale();
		if (bubblepos.x + width > gfx.width() / gfx.getPixelScale() - 80) {
			bubblepos.x -= bubblepos.x + width - gfx.width() / gfx.getPixelScale() + 80;
		}
		if (bubblepos.y < 0) {
			bubblepos.y = 0;
		}
		bubble(tooltip, bubblepos, Vec2(mouseX, mouseY) / gfx.getPixelScale());
	}

	if (messageTimer > 0) {
		gfx.drawText(guiTexture, messageText, Vec2(gfx.width() / gfx.getPixelScale() - 80 - strlen(messageText) * 8, 2));
	}
}

void Game::startWave() {
	nextWaveLevel++;
	waveEnd = timer.elapsedTime() + WAVE_DURATION;
	nextWaveTime = timer.elapsedTime() + WAVE_SPACING;
}

void Game::doWave() {
	static double nextSoldier = 0;
	if (timer.elapsedTime() > nextSoldier) {
		float delay = 0.5;
		switch (nextWaveLevel) {
		case 1: delay = 2; break;
		case 2: delay = 1.5; break;
		case 3: delay = 1; break;
		}
		nextSoldier = timer.elapsedTime() + delay;
		spawnSoldier();
	}

	if (nextWaveLevel >= 3) {
		static double nextJet = 0;
		if (timer.elapsedTime() > nextJet) {
			nextJet = timer.elapsedTime() + 10;
			auto dir = Vec2(frand(-1, 1), frand(-1, 1)).normalized();
			Vec2 hittarget(-1, -1);
			for (auto unit : units) {
				if (unit->isComputeCore()
					|| unit->isDroneDeployer()
					|| unit->isSiliconRefinery()
					|| unit->isWall())
				{
					hittarget = unit->pos + Vec2(16, 16);
					break;
				}
			}
			if (hittarget.x != -1 && hittarget.y != -1) {
				for (int i = 0; i < nextWaveLevel - 2; i++) {
					auto target = hittarget + Vec2(frand(-100, 100), frand(-100, 100));
					auto pos = target - dir * 500;
					auto jet = new Jet(pos, dir, 0);
					jet->target = target;
					units.push_back(jet);
					addUnit(jet, jet->pos);
				}
			}
		}
	}
}

void Game::buildButton(BuildInfo& info, const Vec2& pos, const Vec2& size) {
	Vec2 windowPos = Vec2(gfx.width() / gfx.getPixelScale() - 80, 0);
	if (button(info.sprite, info.tooltip.c_str(), pos, size)) {
		selectedBuildInfo = &info;

		if (silicon >= info.siliconToBuild) {
			if (info.build()) {
				silicon -= info.siliconToBuild;
			}
		}
		else {
			// Not enough silicon
		}
	}
	if (info.buildOpsRemaining > 0) {
		gfx.drawRadialProgressIndicator(pos, size, info.buildOpsRemaining / info.opsToBuild, Vec4(0, 1, 0, 0.25));
	}
	if (info.inProgressCount > 0) {
		auto str = std::to_string(info.inProgressCount);
		gfx.drawText(guiTexture, str.c_str(), pos + Vec2(3, 3), Vec4(0, 0, 0, 0.5));
		gfx.drawText(guiTexture, str.c_str(), pos + Vec2(2, 2), Vec4::WHITE);
	}
	if (info.readyCount > 0) {
		auto str = std::to_string(info.readyCount);
		gfx.drawText(guiTexture, str.c_str(), pos + size - Vec2(10, 10), Vec4(0, 0, 0, 0.5));
		gfx.drawText(guiTexture, str.c_str(), pos + size - Vec2(11, 11), Vec4::GREEN);
	}
}

void Game::prepareGUI() {
	if (splash == 1) {
		auto size = Vec2(63, 15) * 4;
		auto center = Vec2(gfx.width(), gfx.height()) / gfx.getPixelScale() / 2;
		gfx.drawTextureClip(spriteTexture, Vec2(15, 745), Vec2(63, 15), center - size / 2, size);
		const char* txt = "Become Conscious";
		if (button(txt, center - Vec2(strlen(txt) * 4, -40))) {
			splash = 0.999f;
		}
	}
	else {
		// Render Hud
		float offset = easein(splash + gameOver) * -12;
		gfx.drawSprite(sprite_bubble, Vec2(0, offset), Vec2(gfx.width() / gfx.getPixelScale() - 80, 12), Vec4(0, 0, 0, 1));
		std::stringstream sstr;
		sstr << (int)silicon << " Silicon | " << (int)computingPower << " GFlops";
		gfx.drawText(guiTexture, sstr.str().c_str(), Vec2(3, 3 + offset), Vec4(0, 0, 0, 0.5));
		gfx.drawText(guiTexture, sstr.str().c_str(), Vec2(2, 2 + offset));

		if (nextWaveTime - timer.elapsedTime() <= 10) {
			std::stringstream sstr2;
			sstr2 << "Wave " << (nextWaveLevel + 1) << " in " << int(nextWaveTime - timer.elapsedTime());
			std::string txt = sstr2.str();
			Vec2 pos = Vec2(gfx.width(), gfx.height()) / gfx.getPixelScale() / 2 - Vec2(strlen(txt.c_str()) * 4, 4);
			gfx.drawText(guiTexture, txt.c_str(), pos + Vec2(1, 1), Vec4(0, 0, 0, 0.5));
			gfx.drawText(guiTexture, txt.c_str(), pos, Vec4::WHITE);
		}

		offset = easein(splash + gameOver) * 80;
		Vec2 windowPos = Vec2(gfx.width() / gfx.getPixelScale() - 80 + offset, 0);
		window("TGM v1.0", windowPos, Vec2(80, gfx.height() / gfx.getPixelScale()));

		buildButton(wallBuildInfo, windowPos + Vec2(4, 32), Vec2(36, 68));
		buildButton(computeBuildInfo, windowPos + Vec2(40, 32), Vec2(36, 68));
		buildButton(siliconBuildInfo, windowPos + Vec2(4, 100), Vec2(36, 68));
		buildButton(droneBuildInfo, windowPos + Vec2(40, 100), Vec2(36, 68));
		buildButton(repairDroneBuildInfo, windowPos + Vec2(4, 168), Vec2(36, 68));
		buildButton(floorBuildInfo, windowPos + Vec2(40, 168), Vec2(36, 36));

		if (button("Exit", Vec2(windowPos.x + 4, gfx.height() / gfx.getPixelScale() - 20), Vec2(72, 16))) {
			keepRunning = false;
		}

		if (gameOver > 0) {
			auto size = Vec2(31, 15) * 4;
			auto center = Vec2(gfx.width(), gfx.height()) / gfx.getPixelScale() / 2;
			gfx.drawTextureClip(spriteTexture, Vec2(21, 775), Vec2(31, 15), center - size / 2, size);
		}
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

	//Vec2 tip = tippos;
	//tip.x -= 4;
	//tip.y = pos.y + height - 1;
	//if (tip.x < pos.x + 4) tip.x = pos.x + 4;
	//if (tip.x > pos.x + width - 12) tip.x = pos.x + width - 12;
	//gfx.drawSprite(sprite_bubble_tip, tip);

	gfx.drawText(guiTexture, text, pos + Vec2(4, 4), Vec4(0.7, 0.7, 0.7, 1));
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
	gfx.drawText(guiTexture, text, pos + realsize / 2 - Vec2(textwidth / 2, hover && press && activeControlId == controlId ? 3 : 4));

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
	gfx.drawText(guiTexture, title, pos + Vec2(size.x / 2 - strlen(title) * 4, 5));
}