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
#include "Game.h"
#include "Gfx.h"
#include "Sfx.h"
#include "AudioClip.h"

Sprite Soldier::sprites[6];

void Soldier::findTarget(Game& game) {
	target = nullptr;
	float distance = 9999999999;
	for (auto unit : game.getUnits()) {
		if ((unit->isComputeCore() || unit->isWall() || unit->isDroneDeployer() || unit->isSiliconRefinery()) && (!target || ((unit->pos - pos).squaredLength() < distance))) {
			target = unit;
			distance = (unit->pos - pos).squaredLength();
		}
	}
}

void Soldier::update(float dt, Game& game, Sfx& sfx) {
	time += dt;
	if (time > 1) time = 0;
	shoottime -= dt;

	switch (state) {
	case STAND:
		findTarget(game);
		if (target) state = RUN;
		break;
	case RUN:
		if (!target->isAlive()) {
			target = nullptr;
			state = STAND;
			break;
		}
		auto tpos = target->pos + Vec2(16, 16);
		auto vel = (tpos - pos).normalized() * 10;
		mirrored = vel.x > 0;
		auto newpos = pos + vel * dt;
		pos = newpos;
		if ((tpos - pos).length() < 32) state = SHOOT;
		break;
	case SHOOT:
		if (!target->isAlive()) {
			target = nullptr;
			state = STAND;
			break;
		}
		if (shoottime < 0) {
			if (grenadier) {
				game.spawnGrenade(pos, target->pos + Vec2(16, 16), Faction::CPU);
				shoottime = frand(2, 4);
			}
			else {
				target->damage(damage_bullet, Faction::CPU);
				sfx.play(sfx.getAudioClip("media/sounds/gun_burst.wav", 2), 0.5f, 0.0f, frand(0.9, 1.1));
				shoottime = frand(1, 3);
			}
		}
		break;
	}
}

void Soldier::draw_bottom(Gfx& gfx) {
	int frame = 0;
	switch (state) {
	case STAND: frame = 5; break;
	case RUN: frame = int(time * 8) % 4; break;
	case SHOOT: frame = shoottime < 0.5f ? 4 + int(time * 16) % 2 : 5; break;
	}
	gfx.drawSprite(sprites[frame], pos + Vec2(-5, -4) - floor(cameraPosition), Vec4::WHITE, mirrored);
}

void Soldier::damage(int amount, Faction originator) {
	if (originator == Faction::CPU) return;
	health -= amount;
	if (health <= 0) alive = false;
}
