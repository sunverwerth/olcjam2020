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

#include "AudioClip.h"
#include "sys.h"
#include <fstream>
#include <cstdint>

AudioClip::AudioClip(const char* filename) {
	std::ifstream file(filename, std::ios::binary);
	if (!file.good()) {
		log_error("Can not open audio clip %s.", filename);
		return;
	}

	uint32_t dw;
	file.read((char*)&dw, 4); // 'RIFF'
	file.read((char*)&dw, 4); // FILESIZE - 8
	file.read((char*)&dw, 4); // 'WAVE'
	file.read((char*)&dw, 4); // 'fmt '
	file.read((char*)&dw, 4); // fmt length
	uint16_t fmt_tag;
	file.read((char*)&fmt_tag, 2);
	uint16_t channels;
	file.read((char*)&channels, 2);
	uint32_t sampleRate;
	file.read((char*)&sampleRate, 4);
	uint32_t bytesPerSec;
	file.read((char*)&bytesPerSec, 4);
	uint16_t blockAlign;
	file.read((char*)&blockAlign, 2);
	uint16_t bitsPerSample;
	file.read((char*)&bitsPerSample, 2);

	uint32_t dataLength;
	while (file.good()) {
		file.read((char*)&dw, 4); // 'data'
		file.read((char*)&dataLength, 4); // data length
		if (dw == 'atad') break;
		file.seekg(dataLength, std::ios::cur);
	}

	if (!file.good()) {
		log_error("Unexpected end of audio file %s.", filename);
		return;
	}

	numChannels_ = channels;
	samplesPerSecond_ = sampleRate;
	numSamples_ = dataLength / numChannels_ / (bitsPerSample / 8);
	samples = new float[numSamples_ * numChannels_];
	float* write = samples;
	for (int i = 0; i < numSamples_; i++) {
		for (int c = 0; c < numChannels_; c++) {
			short v;
			file.read((char*)&v, 2);
			*write++ = float(v) / 65536;
		}
	}
}

AudioClip::~AudioClip() {
	delete[] samples;
}

int AudioClip::render(float* buffer, int numSamples, int nextSample, float volume, float pan, float pitch, bool loop) {
	float l = (pan > 0 ? 1 - pan : 1) * volume;
	float r = (pan < 0 ? 1 + pan : 1) * volume;

	float playbackPointer = nextSample;
	float advance = pitch * samplesPerSecond_ / 48000;

	if (numChannels_ == 1) {
		for (int i = 0; i < numSamples; i++) {
			int sample = (int)playbackPointer;
			*buffer++ += samples[sample] * l;
			*buffer++ += samples[sample] * r;
			
			playbackPointer += advance;
			if (playbackPointer >= numSamples_) {
				if (!loop) break;
				float intpart;
				playbackPointer = modf(playbackPointer, &intpart);
			}
		}
	}
	else if (numChannels_ == 2) {
		for (int i = 0; i < numSamples; i++) {
			int sample = (int)playbackPointer;
			*buffer++ += samples[sample * 2] * l;
			*buffer++ += samples[sample * 2 + 1] * r;

			playbackPointer += advance;
			if (playbackPointer >= numSamples_) {
				if (!loop) break;
				float intpart;
				playbackPointer = modf(playbackPointer, &intpart);
			}
		}
	}
	return (int)playbackPointer;
}
