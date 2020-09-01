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

#include <SDL2/SDL.h>
#include <map>
#include <string>
#include <vector>

class AudioSource;
class AudioClip;
class AudioTrack;

class Sfx {
public:
	Sfx();
	~Sfx();

	AudioClip* getAudioClip(const char* filename, int maxRef = -1);
	AudioTrack* play(AudioSource*, float volume = 1.0f, float pan = 0.0f, float pitch = 1.0f, bool loop = false);
	AudioTrack* loop(AudioSource*, float volume = 1.0f, float pan = 0.0f, float pitch = 1.0f);

private:
	static void SDLCALL audioCallback(void* userdata, unsigned char* stream, int len);

private:
	std::vector<AudioTrack*> audioTracks;
	std::map<std::string, AudioClip*> audioClips;
	SDL_AudioDeviceID device{ 0 };
	SDL_AudioSpec spec{};
};
