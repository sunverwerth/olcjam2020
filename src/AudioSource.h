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

class AudioSource {
public:
	AudioSource(int maxRef): maxRef(maxRef) {}
	virtual ~AudioSource() {}
	virtual int render(float* buffer, int numSamples, int nextSample, float volume, float pan, float pitch, bool loop) = 0;
	virtual int numSamples() const = 0;
	virtual int numChannels() const = 0;
	virtual int samplesPerSecond() const = 0;
	void release() { refCount--; }
	void addRef() { refCount++; }
	int getRef() const { return refCount; }
	int getMaxRef() const { return maxRef; }

private:
	int refCount{ 0 };
	int maxRef{ -1 };
};
