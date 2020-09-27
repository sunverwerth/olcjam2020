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

#include "sys.h"
#include "Sfx.h"
#include "AudioClip.h"
#include "AudioTrack.h"
#include <SDL2/SDL.h>

Sfx::Sfx() {
	const char* deviceName = nullptr;
	SDL_AudioSpec desired{};
	desired.callback = audioCallback;
	desired.channels = 2;
	desired.format = AUDIO_F32SYS;
	desired.freq = 48000;
	desired.samples = 4096;
	desired.userdata = this;
	device = SDL_OpenAudioDevice(deviceName, 0, &desired, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
	if (!device) {
		sys_crash("Could not initialize audio device.");
	}

	SDL_PauseAudioDevice(device, 0);
}

Sfx::~Sfx() {
	SDL_PauseAudioDevice(device, 1);
	SDL_CloseAudioDevice(device);
}

AudioClip* Sfx::getAudioClip(const char* filename, int maxRef) {
	auto it = audioClips.find(filename);
	if (it != audioClips.end()) return it->second;

	auto clip = new AudioClip(filename, maxRef);
	audioClips[filename] = clip;
	return clip;
}

AudioTrack* Sfx::play(AudioSource* clip, float volume, float pan, float pitch, bool loop) {
	SDL_LockAudioDevice(device);
	
	AudioTrack* track = nullptr;
	if (clip->getMaxRef() <= 0 || clip->getRef() < clip->getMaxRef()) {
		clip->addRef();
		for (auto existingTrack : audioTracks) {
			if (existingTrack->isFree()) {
				track = existingTrack;
				break;
			}
		}

		if (!track) {
			track = new AudioTrack(clip, volume, pan, pitch, loop);
			audioTracks.push_back(track);
		}
		else {
			track->source = clip;
			track->volume = volume;
			track->pan = pan;
			track->pitch = pitch;
			track->loop = loop;
			track->nextSample = 0;
		}
	}

	SDL_UnlockAudioDevice(device);
	return track;
}

AudioTrack* Sfx::loop(AudioSource* clip, float volume, float pan, float pitch) {
	return play(clip, volume, pan, pitch, true);
}

void SDLCALL Sfx::audioCallback(void* userdata, unsigned char* stream, int len) {
	memset(stream, 0, len);

	auto self = reinterpret_cast<Sfx*>(userdata);
	auto samples = reinterpret_cast<float*>(stream);
	auto numSamples = len / (sizeof(float) * self->spec.channels);

	for (auto track : self->audioTracks) {
		if (track->isFinished()) {
			track->cleanUp();
			continue;
		}

		track->render(samples, numSamples);
	}
}