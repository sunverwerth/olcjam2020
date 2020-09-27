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

#include "Level.h"
#include "sys.h"
#include <fstream>

std::vector<Unit*> emptyVector;

Level::Level(int width, int height) : width_(width), height_(height), tiles(new int[width * height]), structures(new int[width * height]), unitsOnTile(new std::vector<Unit*>[width * height]) {
	memset(tiles, 0, sizeof(int) * width * height);
	memset(structures, -1, sizeof(int) * width * height);
}

Level::~Level() {
	delete[] unitsOnTile;
	delete[] tiles;
	delete[] structures;
}

int Level::getTile(int x, int y) const {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return 0;
	return tiles[y * width_ + x];
}

void Level::setTile(int x, int y, int tile) {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

	tiles[y * width_ + x] = tile;
}

int Level::getStructure(int x, int y) const {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return 0;
	return structures[y * width_ + x];
}

void Level::setStructure(int x, int y, int tile) {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

	structures[y * width_ + x] = tile;
}

std::vector<Unit*>& Level::getUnits(int x, int y) const
{
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return emptyVector;

	return unitsOnTile[y * width_ + x];
}

void Level::addUnit(int x, int y, Unit* unit)
{
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

	unitsOnTile[y * width_ + x].push_back(unit);
}

void Level::removeUnit(int x, int y, Unit* unit)
{
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

	auto& vector = unitsOnTile[y * width_ + x];
	vector.erase(std::find(vector.begin(), vector.end(), unit));
}

void Level::load() {
	std::ifstream file("media/level.dat");
	if (!file.good()) {
		log_error("Could not open level file for reading.");
		return;
	}
	file.read(reinterpret_cast<char*>(tiles), sizeof(int) * (size_t)width_ * (size_t)height_);
	file.read(reinterpret_cast<char*>(structures), sizeof(int) * (size_t)width_ * (size_t)height_);
	for (int i = 0; i < width_ * height_; i++) {
		unitsOnTile[i].clear();
	}
}

void Level::save() const {
	std::ofstream file("media/level.dat");
	if (!file.good()) {
		log_error("Could not open level file for writing.");
		return;
	}
	file.write(reinterpret_cast<const char*>(tiles), sizeof(int) * width_ * height_);
	file.write(reinterpret_cast<const char*>(structures), sizeof(int) * width_ * height_);
}
