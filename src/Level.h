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

#include <vector>

class Unit;

class Level {
public:
	Level(int width, int height);
	~Level();

	int width() const { return width_; }
	int height() const { return height_; }

	int getTile(int x, int y) const;
	void setTile(int x, int y, int tile);
	int getStructure(int x, int y) const;
	void setStructure(int x, int y, int structure);
	std::vector<Unit*>& getUnits(int x, int y) const;
	void addUnit(int x, int y, Unit* unit);
	void removeUnit(int x, int y, Unit* unit);

	void load();
	void save() const;

private:
	int width_;
	int height_;
	int* tiles;
	int* structures;
	std::vector<Unit*>* unitsOnTile;
};

