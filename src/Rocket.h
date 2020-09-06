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

#pragma once

#include "Unit.h"
#include "Sprite.h"

class Rocket: public Unit {
public:
	Rocket(const Vec2& pos, const Vec2& target, float speed, Faction faction): Unit(pos, 0), target(target), speed(speed), faction(faction) {}
	virtual void update(float dt, Game& game, Sfx& sfx) override;
	virtual void draw_top(Gfx& gfx) override;
	virtual void draw_bottom(Gfx& gfx) override;

public:
	static Sprite sprite;

private:
	Vec2 target;
	float speed;
	float height{ 32 };
	Faction faction;
};
