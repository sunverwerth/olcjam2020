#include "Rocket.h"
#include "globals.h"
#include "Gfx.h"
#include "Game.h"

Sprite Rocket::sprites[4];

void Rocket::update(float dt, Game& game, Sfx& sfx) {
	speed += dt * 100;
	if (speed > 300) speed = 300;

	auto distance = target - pos;
	auto vel = distance.normalized() * speed;
	pos += vel * dt;

	if (distance.length() < 10) {
		alive = false;
		game.spawnExplosion(pos);
	}
}

void Rocket::draw(Gfx& gfx) {
	auto distance = target - pos;
	auto vel = distance.normalized() * speed;
	gfx.drawRotatedSprite(sprites[1], pos - round(cameraPosition), atan2(vel.y, vel.x));
}

void Rocket::takeExplosionDamage() {
}

bool Rocket::isAlive() {
    return alive;
}
