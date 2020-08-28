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

struct Vec2 {
    Vec2() = default;
    Vec2(float x, float y): x(x), y(y) {}
    explicit Vec2(float v): x(v), y(v) {}
    Vec2(const Vec2& other): x(other.x), y(other.y) {}

    Vec2 operator+(const Vec2& rhs) const {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator-(const Vec2& rhs) const {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator*(const Vec2& rhs) const {
        return Vec2(x * rhs.x, y * rhs.y);
    }

    Vec2 operator/(const Vec2& rhs) const {
        return Vec2(x / rhs.x, y / rhs.y);
    }

    Vec2 operator*(float rhs) const {
        return Vec2(x * rhs, y * rhs);
    }

    Vec2 operator/(float rhs) const {
        return Vec2(x / rhs, y / rhs);
    }

    float length() const {
        return std::sqrt(squaredLength());
    }

    float squaredLength() const {
        return x * x + y * y;
    }

    Vec2 normalized() const {
        auto l = length();
        return Vec2(x / l, y / l);
    }

    void normalize() {
        *this /= length();
    }

    Vec2& operator+=(const Vec2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    Vec2& operator-=(const Vec2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    Vec2& operator*=(const Vec2& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    Vec2& operator/=(const Vec2& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    Vec2& operator*=(float v) {
        x *= v;
        y *= v;
        return *this;
    }

    Vec2& operator/=(float v) {
        x /= v;
        y /= v;
        return *this;
    }

    float x;
    float y;
};
