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

#include "DroneDeployer.h"
#include "utils.h"
#include "Game.h"
#include "Gfx.h"
#include "globals.h"
#include <algorithm>
#include "Drone.h"

Sprite DroneDeployer::sprites[4];

DroneDeployer::DroneDeployer(const Vec2& pos) : Unit(pos, 500) {
	animSpeed = frand(1, 1.5);
}

void DroneDeployer::update(float dt, Game& game, Sfx& sfx) {
	if (!drone) {
		drone = game.spawnDrone(pos + Vec2(16, 16), repair);
		drone->origin = this;
	}

	time += dt * animSpeed;
	damageTime -= dt;
	healTime -= dt;
	if (damageTime < 0) damageTime = 0;
	if (health <= 0) {
		game.spawnExplosion(pos + Vec2(16, 16), false, Faction::Player);
		alive = false;
	}
	checkEnemyTime -= dt;
	if (checkEnemyTime < 0 && numDrones > 0) {
		checkEnemyTime = 3;
		if (repair) {
			float mindist = 9999999999;
			for (auto building : game.getUnits()) {
				float dsq = (building->pos - pos).squaredLength();
				if (building->isPlayerStructure() && !building->hasFullHealth() && (pos - building->pos).length() < 300 && dsq < mindist) {
					drone->target = building;
					mindist = dsq;
				}
			}
		}
		else {
			for (auto soldier : game.getUnits()) {
				if (soldier->isSoldier() && (pos - soldier->pos).length() < 300) {
					drone->target = soldier;
					break;
				}
			}
		}
	}
}

void DroneDeployer::draw_structure(Gfx& gfx) {
	int frame = int(time * 8) % 2;
	if (repair) frame += 2;
	Vec4 color = Vec4::WHITE;
	if (damageTime > 0) color = Vec4(1 + damageTime, 1 + damageTime, 1, 1);
	if (healTime > 0) color = Vec4(1, 1, 1 + healTime, 1);
	gfx.drawSprite(sprites[frame], pos - floor(cameraPosition), color);
}

void DroneDeployer::draw_top(Gfx& gfx) {
	if (damageTime > 0 || healTime > 0) {
		gfx.drawTextureClip(sprites[0].texture, Vec2(100, 100), Vec2(1, 1), pos - floor(cameraPosition) - Vec2(1, 11), Vec2(34, 4), Vec4::BLACK);
		gfx.drawTextureClip(sprites[0].texture, Vec2(100, 100), Vec2(1, 1), pos - floor(cameraPosition) - Vec2(0, 10), Vec2(32 * health / maxHealth, 2), Vec4(0, 0.7, 0, 1));
	}
}

void DroneDeployer::damage(int amount, Faction originator) {
	if (originator == Faction::Player) return;
	health -= amount;
	damageTime = 0.5;
}

void DroneDeployer::heal(float amount) {
	Unit::heal(amount);
	healTime = 0.5;
}
