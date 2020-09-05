/*
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

#include "Soldier.h"
#include "utils.h"
#include "globals.h"
#include "Gfx.h"
#include "Sfx.h"
#include "AudioClip.h"

Sprite Soldier::sprites[6];

void Soldier::update(float dt, Game& game, Sfx& sfx) {
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

void Soldier::draw(Gfx& gfx) {
	int frame = 0;
	switch (state) {
	case STAND: frame = 5; break;
	case RUN: frame = int(time * 8) % 4; break;
	case SHOOT: frame = 4 + int(time * 16) % 2; break;
	}
	gfx.drawSprite(sprites[frame], pos + Vec2(-5, -4) - round(cameraPosition), Vec4::WHITE, target.x > pos.x);
}

void Soldier::takeExplosionDamage() {
	alive = false;
}

bool Soldier::isAlive() {
	return alive;
}
