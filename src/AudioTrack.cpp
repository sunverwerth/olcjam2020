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

#include "AudioTrack.h"
#include "AudioClip.h"
#include <cstdlib>

bool AudioTrack::isFinished() const {
	if (source == nullptr) return true;
	if (!loop && nextSample >= source->numSamples()) return true;
	return false;
}

bool AudioTrack::isFree() const {
	return source == nullptr;
}

void AudioTrack::cleanUp() {
	if (source) {
		source->release();
		source = nullptr;
	}
}

void AudioTrack::render(float* buffer, int numSamples) {
	nextSample = source->render(buffer, numSamples, nextSample, volume, pan, pitch, loop);
}