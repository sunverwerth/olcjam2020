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

#include "Vec2.h"

class Sfx;
class Gfx;
class Game;

enum DamageType {
	DAMAGE_BULLET,
	DAMAGE_EXPLOSION,
	DAMAGE_EXPLOSION_SMALL,
};

class Unit {
public:
	Unit(const Vec2& pos) : pos(pos) {}
	virtual ~Unit() {}
	virtual void update(float dt, Game& game, Sfx& sfx) {};
	virtual void draw_floor(Gfx& gfx) {};
	virtual void draw_structure(Gfx& gfx) {};
	virtual void draw_bottom(Gfx& gfx) {};
	virtual void draw_top(Gfx& gfx) {};
	virtual void damage(DamageType) {};
	bool isAlive() const { return alive; }
	bool inRadius(const Vec2& c, float r) {
		return (c - pos).length() < r;
	}

	virtual bool isSoldier() const { return false; }
	virtual bool isComputeCore() const { return false; }
	virtual bool isWall() const { return false; }
	virtual bool isDroneDeployer() const { return false; }
	virtual bool isSiliconRefinery() const { return false; }
	virtual bool isCrater() const { return false; }

public:
	Vec2 pos;
	bool alive{ true };
};
