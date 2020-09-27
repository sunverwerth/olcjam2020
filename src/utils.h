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

#include <cstdlib>

static inline int countNewlines(const char* text) {
	int nl = 0;
	char ch;
	while (ch = *text++) {
		if (ch == '\n') nl++;
	}
	return nl;
}

static inline int longestLine(const char* text) {
	int longest = 0;
	int current = 0;
	char ch;
	while (ch = *text++) {
		if (ch == '\n') current = 0;
		current++;
		if (current > longest) longest = current;
	}
	return longest;
}

static inline float clamp(float  v, float  min, float  max) {
	if (v < min) return min;
	if (v > max) return max;
	return v;
}

static inline float frand(float min, float max) {
	return min + (max - min) * rand() / RAND_MAX;
}

static inline float easein(float t) {
	return t * t;
}

static inline float easeout(float t) {
	return 1 - easein(1 - t);
}