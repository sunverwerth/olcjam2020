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

#include <cmath>

struct Vec3 {
	Vec3() = default;
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	explicit Vec3(float v) : x(v), y(v), z(v) {}
	Vec3(const Vec3& other) : x(other.x), y(other.y), z(other.z) {}

	bool operator==(const Vec3& rhs) const {
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}

	Vec3 operator+(const Vec3& rhs) const {
		return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	Vec3 operator-(const Vec3& rhs) const {
		return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	Vec3 operator*(const Vec3& rhs) const {
		return Vec3(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	Vec3 operator/(const Vec3& rhs) const {
		return Vec3(x / rhs.x, y / rhs.y, z / rhs.z);
	}

	Vec3 operator*(float rhs) const {
		return Vec3(x * rhs, y * rhs, z * rhs);
	}

	Vec3 operator/(float rhs) const {
		return Vec3(x / rhs, y / rhs, z / rhs);
	}

	Vec3 operator-() const {
		return Vec3(-x, -y, -z);
	}

	float length() const {
		return std::sqrt(squaredLength());
	}

	float squaredLength() const {
		return x * x + y * y + z * z;
	}

	Vec3 normalized() const {
		auto l = length();
		return Vec3(x / l, y / l, z / l);
	}

	void normalize() {
		*this /= length();
	}

	float dot(const Vec3& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	Vec3 cross(const Vec3& other) const {
		return Vec3(
			y * other.z - z * other.y,
			-x * other.z + z * other.x,
			x * other.y - y * other.x
		);
	}

	Vec3 projectOnPlane(const Vec3& normal) const {
		return *this - normal * normal.dot(*this);
	}

	Vec3 reflect(const Vec3& normal, float amount = 1.0f) const {
		return *this - normal * normal.dot(*this) * 2 * amount;
	}

	Vec3& operator=(const Vec3& rhs) {
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		return *this;
	}

	Vec3& operator+=(const Vec3& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	Vec3& operator-=(const Vec3& rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	Vec3& operator*=(const Vec3& rhs) {
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		return *this;
	}

	Vec3& operator/=(const Vec3& rhs) {
		x /= rhs.x;
		y /= rhs.y;
		z /= rhs.z;
		return *this;
	}

	Vec3& operator*=(float v) {
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}

	Vec3& operator/=(float v) {
		x /= v;
		y /= v;
		z /= v;
		return *this;
	}

	float x;
	float y;
	float z;

	const static Vec3 ONE;
	const static Vec3 ZERO;
	const static Vec3 UP;
	const static Vec3 DOWN;
	const static Vec3 LEFT;
	const static Vec3 RIGHT;
	const static Vec3 FORWARD;
	const static Vec3 BACK;
};

const Vec3 Vec3::ONE(1, 1, 1);
const Vec3 Vec3::ZERO(0, 0, 0);
const Vec3 Vec3::UP(0, 1, 0);
const Vec3 Vec3::DOWN(0, -1, 0);
const Vec3 Vec3::LEFT(-1, 0, 0);
const Vec3 Vec3::RIGHT(1, 0, 0);
const Vec3 Vec3::FORWARD(0, 0, 1);
const Vec3 Vec3::BACK(0, 0, -1);

float dot(const Vec3& l, const Vec3& r) {
	return l.x * r.x + l.y * r.y + l.z * r.z;
}
