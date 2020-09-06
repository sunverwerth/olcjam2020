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

#include "Rocket.h"
#include "globals.h"
#include "Gfx.h"
#include "Game.h"
#include "utils.h"

Sprite Rocket::sprite;

void Rocket::update(float dt, Game& game, Sfx& sfx) {
	speed += dt * 100;
	if (speed > 300) speed = 300;

	auto distance = target - pos;
	auto vel = distance.normalized() * speed;
	pos += vel * dt;

	height = clamp(height - dt * 100, 8, 32);

	if (distance.length() < 10) {
		alive = false;
		game.spawnExplosion(pos, false, faction);
	}
}

void Rocket::draw_top(Gfx& gfx) {
	auto distance = target - pos;
	auto vel = distance.normalized() * speed;
	gfx.drawRotatedSprite(sprite, pos - floor(cameraPosition) + Vec2(0, -height), atan2(vel.y, vel.x));
}

void Rocket::draw_bottom(Gfx& gfx) {
	auto distance = target - pos;
	auto vel = distance.normalized() * speed;
	gfx.drawRotatedSprite(sprite, pos - floor(cameraPosition), atan2(vel.y, vel.x), Vec4(0, 0, 0, 0.5));
}
