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

#include "Jet.h"
#include "globals.h"
#include "Gfx.h"
#include "Game.h"
#include "utils.h"
#include "Grenade.h"
#include "Sfx.h"
#include "AudioClip.h"

Sprite Jet::sprites[2];

void Jet::update(float dt, Game& game, Sfx& sfx) {
	if (time == 0) {
		sfx.play(sfx.getAudioClip("media/sounds/jet.wav", 1), 1, 0, frand(0.9, 1.1));
	}

	speed += dt * 100;
	if (speed > 200) speed = 200;

	time += dt;

	pos += dir * speed * dt;
	drop -= dt;
	if ((pos-target).length() < 150 && drop < 0) {
		drop = frand(0.05, 0.1);
		auto grenade = game.spawnGrenade(pos, pos, Faction::CPU);
		grenade->time = 0.5f;
	}

	if (pos.x < 0 || pos.y < 0 || pos.x > 3000 || pos.y > 3000) {
		alive = false;
	}
}

void Jet::draw_top(Gfx& gfx) {
	int frame = int(time * 10) % 2;
	gfx.drawRotatedSprite(sprites[frame], pos - floor(cameraPosition) + Vec2(0, -32), atan2(dir.y, dir.x));
}

void Jet::draw_bottom(Gfx& gfx) {
	int frame = int(time * 10) % 2;
	gfx.drawRotatedSprite(sprites[frame], pos - floor(cameraPosition), atan2(dir.y, dir.x), Vec4(0, 0, 0, 0.5f));
}
