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

#include "Grenade.h"
#include "globals.h"
#include "Gfx.h"
#include "Game.h"
#include "utils.h"

Sprite Grenade::sprite;

Grenade::Grenade(const Vec2& pos, const Vec2& target, Faction faction) : Unit(pos, 0), target(target), rotation(frand(-20, 20)), faction(faction) {}

void Grenade::update(float dt, Game& game, Sfx& sfx) {
	time += dt;
	if (time > 1) {
		alive = false;
		game.spawnExplosion(target, true, faction);
	}
}

void Grenade::draw_top(Gfx& gfx) {
	float height = sin(time * 3.14159) * 32;
	gfx.drawRotatedSprite(sprite, pos + (target - pos) * time - floor(cameraPosition) + Vec2(0, -height), time * rotation);
}

void Grenade::draw_bottom(Gfx& gfx) {
	gfx.drawRotatedSprite(sprite, pos + (target - pos) * time - floor(cameraPosition), time * rotation, Vec4(0, 0, 0, 0.5));
}
