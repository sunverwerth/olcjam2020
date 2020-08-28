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

#include "log.h"
#include "sys.h"
#include "Gfx.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

class Gfx;
class Timer;

class Game {
public:
	Game(Gfx& gfx, Timer& timer): gfx(gfx), timer(timer) {}
	void handleEvent(const SDL_Event&);
	bool shouldKeepRunning() const { return keepRunning; }
	void prepareFrame();

private:
	bool keepRunning{ true };
	Gfx& gfx;
	Timer& timer;
};

void Game::handleEvent(const SDL_Event& event) {
	if (event.type == SDL_QUIT) {
		keepRunning = false;
		return;
	}
}

void Game::prepareFrame() {}

class Timer {
public:
	Timer();
	void lap();

	float deltaTime() const { return dt; }
	double elapsedTime() const { return time; }

private:
	unsigned int startTick;
	unsigned int lapTick;
	float dt{ 0 };
	double time{ 0 };
};

Timer::Timer() : startTick(SDL_GetTicks()), lapTick(startTick) {}

void Timer::lap() {
	unsigned int tick = SDL_GetTicks();
	dt = (tick - lapTick) / 1000.0f;
	time = (tick - startTick) / 1000.0;
	lapTick = tick;
}

#ifdef _WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main(int argc, char** argv) {
#endif
	log_init();
	sys_init();

	Gfx gfx("OLC CodeJam 2020", 1280, 800, false);
	Timer timer;
	Game game(gfx, timer);

	SDL_Event event;
	bool run = true;
	while (game.shouldKeepRunning()) {
		while (SDL_PollEvent(&event)) {
			game.handleEvent(event);
		}
		timer.lap();
		gfx.beginFrame();
		game.prepareFrame();
		gfx.renderFrame();
	}

	sys_shutdown();
	log_shutdown();
	return 0;
}