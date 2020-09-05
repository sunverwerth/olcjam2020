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

#pragma once

#include "Unit.h"
#include "Sprite.h"

class Soldier : public Unit {
	enum State {
		STAND,
		RUN,
		SHOOT,
	};

public:
	Soldier(const Vec2& pos) : Unit(pos) {}
	virtual void update(float dt, Game& game, Sfx& sfx) override;
	virtual void draw(Gfx& gfx) override;
	virtual void takeExplosionDamage() override;
	virtual bool isAlive() override;
	virtual bool isSoldier() const override { return true; }

public:
	static Sprite sprites[6];

private:
	bool alive{ true };
	State state{ STAND };
	Vec2 target{ 0, 0 };
	float time{ 0 };
	float shoottime{ 0 };
};