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

class DroneDeployer;

class Drone : public Unit {
	enum State {
		WAIT,
		START,
		REPAIR,
		ATTACK,
		RETURN,
		LAND
	};
public:
	Drone(const Vec2& pos);
	virtual void update(float dt, Game& game, Sfx& sfx) override;
	void updateAttack(float dt, Game& game, Sfx& sfx);
	void updateRepair(float dt, Game& game, Sfx& sfx);
	virtual void draw_top(Gfx& gfx) override;
	virtual void draw_bottom(Gfx& gfx) override;
	void selfdestruct(Game& game);

public:
	static Sprite sprite;

public:
	Vec2 speed{ 0,0 };
	Unit* target{ nullptr };
	float fireTime{ 0 };
	DroneDeployer* origin{ nullptr };
	int numRockets{ 1 };
	float healthpoints{ 100 };
	float height{ 0 };
	State state{ WAIT };
	bool repair{ false };
};
