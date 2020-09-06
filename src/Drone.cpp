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

#include "Drone.h"
#include "utils.h"
#include "Game.h"
#include "Gfx.h"
#include "globals.h"
#include <algorithm>
#include "DroneDeployer.h"

Sprite Drone::sprite;

Drone::Drone(const Vec2& pos) : Unit(pos), target(nullptr) {
}

void Drone::update(float dt, Game& game, Sfx& sfx) {
	fireTime -= dt;
	if (target && !target->alive) target = nullptr;
	if (origin && !origin->alive) origin = nullptr;

	switch (state) {
	case WAIT:
		if (!origin) {
			alive = false;
			game.spawnExplosion(pos);
			break;
		}
		if (target) {
			state = START;
			origin->numDrones--;
			break;
		}
		break;
	case START:
		height += dt * 20;
		if (height > 32) {
			height = 32;
			state = ATTACK;
			speed = Vec2(frand(-5, 5), frand(-5, 5));
		}
		break;

	case ATTACK:
		if (target) {
			if ((pos - target->pos).length() < 64 && fireTime <= 0) {
				fireTime = 1;
				game.spawnRocket(pos, target->pos, speed.length());
				numRockets--;
				state = RETURN;
				break;
			}

			speed += (target->pos - pos).normalized() * 100 * dt;
			if (speed.length() > 100) speed = speed.normalized() * 100;
			pos += speed * dt;
		}
		else {
			for (auto soldier : game.getUnits()) {
				if (soldier->isSoldier() && (pos - soldier->pos).length() < 300) {
					target = soldier;
					break;
				}
			}
			if (!target) state = RETURN;
		}
		break;

	case RETURN:
		if (origin) {
			if ((pos - origin->pos - Vec2(16, 16)).length() < 4) {
				pos = origin->pos + Vec2(16, 16);
				speed = Vec2(0, 0);
				state = LAND;
				break;
			}

			speed += (origin->pos + Vec2(16, 16) - pos).normalized() * 100 * dt;
			if (speed.length() > 100) speed = speed.normalized() * 100;
			pos += speed * dt;

			auto homedir = origin->pos + Vec2(16, 16) - pos;
			float homedist = homedir.length();
			auto homenorm = homedir / homedist;
			if (homedist < 100) {
				speed = homenorm * homedist;
			}
		}
		break;

	case LAND:
		if (origin) {
			height -= dt * 20;
			if (height < 0) {
				height = 0;
				origin->numDrones++;
				state = WAIT;
			}
		}
		else {
			alive = false;
			game.spawnExplosion(pos);
		}
		break;
	}
}

void Drone::draw_top(Gfx& gfx) {
	float angle = atan2(speed.y, speed.x);

	gfx.drawRotatedSprite(sprite, pos - floor(cameraPosition) - Vec2(0, height), angle);
}

void Drone::draw_bottom(Gfx& gfx) {
	float angle = atan2(speed.y, speed.x);

	gfx.drawRotatedSprite(sprite, pos - floor(cameraPosition), angle, Vec4(0, 0, 0, 0.5));
}
