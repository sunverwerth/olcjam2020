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
#include <vector>

union SDL_Event;
class Gfx;
class Sfx;
class Timer;
class Texture;
struct DustParticle;
struct Sprite;
struct BuildInfo;
class Unit;

class Game {
public:
	Game(Gfx& gfx, Sfx& sfx, Timer& timer) : gfx(gfx), sfx(sfx), timer(timer) {}
	void start();
	void handleEvent(const SDL_Event&);
	bool shouldKeepRunning() const { return keepRunning; }
	void prepareFrame();
	void bubble(const char* text, const Vec2& pos, const Vec2& tippos);
	void createParticle(DustParticle& p);
	bool inViewport(const Vec2& pos) const;
	void anyKeyPressed();
	void spawnRocket(const Vec2& pos, const Vec2& target, float speed);
	void spawnDrone(const Vec2& pos);
	void spawnExplosion(const Vec2& pos);
	void prepareGUI();

	bool isMouseOver(const Vec2& pos, const Vec2& size);

	bool button(const char* text, const Vec2& pos, const Vec2& size = Vec2(0, 0));
	bool button(const Sprite& sprite, const char* tooltip, const Vec2& pos, const Vec2& size = Vec2(0, 0));
	void window(const char* title, const Vec2& pos, const Vec2& size);
	void buildButton(BuildInfo&, const char* text, const Vec2& pos, const Vec2& size);

	void startWave();
	void doWave();
	void spawnSoldier();

	const std::vector<Unit*> getUnits() const { return units; }

private:
	bool keepRunning{ true };
	Gfx& gfx;
	Sfx& sfx;
	Timer& timer;
	Texture* guitexture{ nullptr };
	Texture* sprites{ nullptr };
	unsigned int mouseButtons{ 0 };
	unsigned int mousePressed{ 0 };
	unsigned int mouseReleased{ 0 };
	int mouseX{ 0 };
	int mouseY{ 0 };
	int controlId{ 0 };
	int activeControlId{ 0 };

	const char* tooltip{ nullptr };

	// Game state
	float computingPower{ 0 };
	float silicon{ 0 };

	// Waves
	int nextWaveLevel{ 0 };
	double nextWaveTime{ 0 };
	double waveEnd{ 0 };

	std::vector<Unit*> units;
};
