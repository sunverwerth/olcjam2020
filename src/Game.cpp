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
#include <SDL2/SDL.h>
#include <cmath>
#include <sstream>

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

Vec2 mainCPUPosition;

Sprite sprite_bubble;
Sprite sprite_bubble_tip;
Sprite sprite_button;
Sprite sprite_button_pressed;

Sprite sprite_explosion[3];

Sprite tiles[256];
int numTiles;
int selectedTile;

Sprite structures[256];
int selectedStructure;

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
}

int Level::getStructure(int x, int y) const {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return 0;
	return structures[y * width_ + x];
}

void Level::setStructure(int x, int y, int tile) {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

	structures[y * width_ + x] = tile;
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

const int WAVE_DURATION = 30;
const int WAVE_SPACING = 6;

Level level(100, 100);
Vec2 cameraPosition{ 500,500 };
Vec2 cameraSpeed;

bool moveUp, moveDown, moveLeft, moveRight;
enum class UIMode {
	GAME,
	TILES,
	STRUCTURES,
} uiMode;

Sprite sprite_rocket[4];
Sprite sprite_drone;
Sprite sprite_drone2;

class Unit {
public:
	virtual ~Unit() {}
	virtual void update(float dt, Sfx& sfx) = 0;
	virtual void draw(Gfx& gfx) = 0;
	virtual void takeExplosionDamage() = 0;
	virtual bool isAlive() = 0;
	bool inRadius(const Vec2& c, float r) {
		return (c - pos).length() < r;
	}

public:
	Vec2 pos;
};

Sprite sprite_soldier[6];

class Soldier : public Unit {
	enum State {
		STAND,
		RUN,
		SHOOT,
	};

public:
	virtual void update(float dt, Sfx& sfx) override {
		time += dt;
		if (time > 1) time = 0;
		shoottime += dt;
		if (shoottime > frand(2, 3)) shoottime = 0;

		switch (state) {
		case STAND:
			target = mainCPUPosition;
			state = RUN;
			break;
		case RUN:
			pos += (target - pos).normalized() * 15 * dt;
			if ((target - pos).length() < 32) state = SHOOT;
			break;
		case SHOOT:
			if (shoottime == 0) sfx.play(sfx.getAudioClip("media/sounds/gun_burst.wav", 1), 0.5f, 0.0f, frand(0.9, 1.1));
			break;
		}
	}

	virtual void draw(Gfx& gfx) override {
		int frame = 0;
		switch (state) {
		case STAND: frame = 5; break;
		case RUN: frame = int(time * 8) % 4; break;
		case SHOOT: frame = 4 + int(time * 16) % 2; break;
		}		
		gfx.drawSprite(sprite_soldier[frame], pos + Vec2(-5, -4) - round(cameraPosition), Vec4::WHITE, target.x > pos.x);
	}

	virtual void takeExplosionDamage() override {
		alive = false;
	}

	virtual bool isAlive() override {
		return alive;
	}

private:
	bool alive{ true };
	State state{ STAND };
	Vec2 target;
	float time{ 0 };
	float shoottime{ 0 };
};

std::vector<Unit*> units;

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

	sprite_soldier[0] = { sprites, Vec2(0, 475), Vec2(6, 5) };
	sprite_soldier[1] = { sprites, Vec2(0, 480), Vec2(6, 5) };
	sprite_soldier[2] = { sprites, Vec2(0, 485), Vec2(6, 5) };
	sprite_soldier[3] = { sprites, Vec2(0, 490), Vec2(6, 5) };
	sprite_soldier[4] = { sprites, Vec2(0, 495), Vec2(6, 5) };
	sprite_soldier[5] = { sprites, Vec2(0, 500), Vec2(6, 5) };

	sprite_rocket[0] = { sprites, Vec2(0,508),Vec2(8,4) };
	sprite_rocket[1] = { sprites, Vec2(0,512),Vec2(8,4) };
	sprite_rocket[2] = { sprites, Vec2(8,508),Vec2(4,8) };
	sprite_rocket[3] = { sprites, Vec2(12,508),Vec2(4,8) };

	sprite_drone = { sprites, Vec2(0,516),Vec2(8,8) };
	sprite_drone2 = { sprites, Vec2(8,516),Vec2(8,8) };

	sprite_explosion[0] = { sprites, Vec2(0,524),Vec2(32,32) };
	sprite_explosion[1] = { sprites, Vec2(32,524),Vec2(32,32) };
	sprite_explosion[2] = { sprites, Vec2(64,524),Vec2(32,32) };

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
			else if (uiMode == UIMode::STRUCTURES) selectedStructure = (selectedStructure + 1) % STRUCTURE_COUNT;
			return;
		}
		case SDLK_MINUS: {
			if (uiMode == UIMode::TILES) selectedTile = (selectedTile + numTiles - 1) % numTiles;
			else if (uiMode == UIMode::STRUCTURES) selectedStructure = (selectedStructure + STRUCTURE_COUNT - 1) % STRUCTURE_COUNT;
			return;
		}
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
	float pan = clamp((pos.x - gfx.width() / gfx.getPixelScale() - cameraPosition.x) / gfx.width(), -0.5, 0.5);
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
	auto rpos = round(pos / 32);
	level.setStructure(rpos.x, rpos.y, 13);

	for (auto& unit : units) {
		if (unit->inRadius(pos, 16)) {
			unit->takeExplosionDamage();
		}
	}

	float pan = clamp((pos.x - gfx.width() / gfx.getPixelScale() - cameraPosition.x) / gfx.width(), -0.5, 0.5);
	sfx.play(sfx.getAudioClip("media/sounds/explosion.wav"), 0.5f, pan, frand(0.5, 1.0));

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

void Game::spawnSoldier() {
	auto soldier = new Soldier;
	soldier->pos = mainCPUPosition + Vec2(frand(-1,1),frand(-1,1)).normalized() * 300;
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
	for (auto& unit : units) {
		unit->update(dt, sfx);
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
	for (auto& unit : units) {
		unit->draw(gfx);
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

		gfx.drawSprite(sprite_explosion[frame], expl.position - Vec2(16, 16) - round(cameraPosition));
	}


	// Render drones
	for (auto& drone : drones) {
		if (!drone.alive) continue;

		if ((drone.position - drone.target).length() < 300 && t > drone.fireTime) {
			drone.fireTime = t + 1;
			spawnRocket(drone.position, drone.target, drone.speed.length());
			if (units.size()) {
				drone.target = units[rand() % units.size()]->pos;
			}
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
		auto vel = distance.normalized() * rocket.speed;
		rocket.position += vel * dt;
		if (distance.length() < 10) {
			rocket.alive = false;
			spawnExplosion(rocket.position);
		}
		
		if (abs(vel.x) > abs(vel.y)) {
			if (vel.x > 0) {
				gfx.drawSprite(sprite_rocket[1], rocket.position - round(cameraPosition));
			}
			else {
				gfx.drawSprite(sprite_rocket[0], rocket.position - round(cameraPosition));
			}
		}
		else {
			if (vel.y > 0) {
				gfx.drawSprite(sprite_rocket[3], rocket.position - round(cameraPosition));
			}
			else {
				gfx.drawSprite(sprite_rocket[2], rocket.position - round(cameraPosition));
			}
		}
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

	// Editor
	if (uiMode == UIMode::TILES) {
		gfx.drawText(guitexture, "Place tiles", Vec2(300, 0), Vec4::BLACK);
		gfx.drawSprite(tiles[selectedTile], Vec2(0, 0));

		int x = (mouseX / gfx.getPixelScale() + cameraPosition.x) / 32;
		int y = (mouseY / gfx.getPixelScale() + cameraPosition.y) / 32;
		gfx.drawSprite(tiles[selectedTile], Vec2(x * 32, y * 32) - round(cameraPosition));

		if (mouseButtons & SDL_BUTTON(1)) {
			level.setTile(x, y, selectedTile);
		}
	}
	else if (uiMode == UIMode::STRUCTURES) {
		gfx.drawText(guitexture, "Place structures", Vec2(300, 0), Vec4::BLACK);
		gfx.drawSprite(structures[selectedStructure], Vec2(0, 0));

		int x = (mouseX / gfx.getPixelScale() + cameraPosition.x) / 32;
		int y = (mouseY / gfx.getPixelScale() + cameraPosition.y) / 32;
		const Sprite& structure = structures[selectedStructure];
		gfx.drawSprite(structure, Vec2(x * 32, y * 32 - structure.clipSize.y + 32) - round(cameraPosition));

		if (mouseButtons & SDL_BUTTON(1)) {
			if (level.getStructure(x, y) != selectedStructure) {
				sfx.play(sfx.getAudioClip("media/sounds/thump.wav"));
			}
			level.setStructure(x, y, selectedStructure);
		}
		if (mouseButtons & SDL_BUTTON(3)) {
			level.setStructure(x, y, -1);
		}
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