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

#include "SiliconRefinery.h"
#include "utils.h"
#include "Game.h"
#include "Gfx.h"
#include "globals.h"
#include <algorithm>

Sprite SiliconRefinery::sprites[2];

SiliconRefinery::SiliconRefinery(const Vec2& pos) : Unit(pos) {
	animSpeed = frand(1, 1.5);
}

void SiliconRefinery::update(float dt, Game& game, Sfx& sfx) {
	time += dt * animSpeed;
	damageTime -= dt;
	if (damageTime < 0) damageTime = 0;
	if (health <= 0) {
		game.spawnExplosion(pos + Vec2(16, 16));
		alive = false;
	}
}

void SiliconRefinery::draw_structure(Gfx& gfx) {
	int frame = int(time * 8) % 2;
	gfx.drawSprite(sprites[frame], pos - floor(cameraPosition), Vec4(1 + damageTime, 1 + damageTime, 1, 1));
}

void SiliconRefinery::draw_top(Gfx& gfx) {
	if (damageTime > 0) {
		gfx.drawTextureClip(sprites[0].texture, Vec2(100, 100), Vec2(1, 1), pos - floor(cameraPosition) - Vec2(1, 11), Vec2(34, 4), Vec4::BLACK);
		gfx.drawTextureClip(sprites[0].texture, Vec2(100, 100), Vec2(1, 1), pos - floor(cameraPosition) - Vec2(0, 10), Vec2(32 * health/100, 2), Vec4(0, 0.7, 0, 1));
	}
}

void SiliconRefinery::damage(DamageType type) {
	switch (type) {
	case DAMAGE_BULLET: health -= 1; break;
	case DAMAGE_EXPLOSION: health -= 10; break;
	case DAMAGE_EXPLOSION_SMALL: health -= 5; break;
	}
	damageTime = 0.5;
}
