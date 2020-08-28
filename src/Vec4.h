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

struct Vec4 {
    Vec4() = default;
    Vec4(float x, float y, float z, float w): x(x), y(y), z(z), w(w) {}
    explicit Vec4(float v): x(v), y(v), z(v), w(v) {}
    Vec4(const Vec4& other): x(other.x), y(other.y), z(other.z), w(other.w) {}

    static Vec4 hex(unsigned int val) {
        return Vec4(
            ((val >> 24) & 0xff) / 255.f,
            ((val >> 16) & 0xff) / 255.f,
            ((val >> 8) & 0xff) / 255.f,
            ((val >> 0) & 0xff) / 255.f
        );
    }

    Vec4 operator+(const Vec4& rhs) const {
        return Vec4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    Vec4 operator-(const Vec4& rhs) const {
        return Vec4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }

    Vec4 operator*(const Vec4& rhs) const {
        return Vec4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
    }

    Vec4 operator/(const Vec4& rhs) const {
        return Vec4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
    }

    Vec4 operator*(float rhs) const {
        return Vec4(x * rhs, y * rhs, z * rhs, w * rhs);
    }

    Vec4 operator/(float rhs) const {
        return Vec4(x / rhs, y / rhs, z / rhs, w / rhs);
    }

    double length() const {
        return std::sqrt(squaredLength());
    }

    double squaredLength() const {
        return x * x + y * y * z * z + w * w;
    }

    Vec4 normalized() const {
        auto l = length();
        return Vec4(x / l, y / l, z / l, w / l);
    }

    void normalize() {
        *this /= length();
    }

	Vec4& operator=(const Vec4& rhs) {
		x = rhs.x;
		y = rhs.y;
		z = rhs.z;
		w = rhs.w;
		return *this;
	}

    Vec4& operator+=(const Vec4& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    Vec4& operator-=(const Vec4& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    Vec4& operator*=(const Vec4& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }

    Vec4& operator/=(const Vec4& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }

    Vec4& operator*=(float v) {
        x *= v;
        y *= v;
        z *= v;
        w *= v;
        return *this;
    }

    Vec4& operator/=(float v) {
        x /= v;
        y /= v;
        z /= v;
        w /= v;
        return *this;
    }

    float x;
    float y;
    float z;
    float w;
};
