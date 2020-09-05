#include "Drone.h"
#include "utils.h"
#include "Game.h"
#include "Gfx.h"
#include "globals.h"
#include <algorithm>

Sprite Drone::sprites[2];

Drone::Drone(const Vec2& pos) : Unit(pos), target(Vec2(frand(100, 1000), frand(100, 1000))) {
}

void Drone::update(float dt, Game& game, Sfx& sfx) {
	fireTime -= dt;
	if ((pos - target).length() < 300 && fireTime <= 0) {
		fireTime = 1;
		game.spawnRocket(pos, target, speed.length());
		
		std::vector<const Unit*> soldiers;
		for (const Unit* unit : game.getUnits()) {
			if (unit->isSoldier()) {
				soldiers.push_back(unit);
			}
		}
		if (!soldiers.empty()) {
			std::sort(soldiers.begin(), soldiers.end(), [=](const auto& a, const auto& b) { return (pos - a->pos).length() < (pos - b->pos).length(); });
			target = soldiers[0]->pos;
		}
	}

	speed += (target - pos).normalized() * 100 * dt;
	if (speed.length() > 100) speed = speed.normalized() * 100;
	pos += speed * dt;
}

void Drone::draw(Gfx& gfx) {
	float angle = atan2(speed.y, speed.x);

	gfx.drawRotatedSprite(sprites[0], pos - round(cameraPosition), angle);
}

void Drone::takeExplosionDamage() {
}

bool Drone::isAlive() {
	return alive;
}
