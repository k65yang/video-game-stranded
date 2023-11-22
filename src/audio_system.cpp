#include "audio_system.hpp"
#include <SDL.h>

void AudioSystem::init() {
	//////////////////////////////////////
// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		assert(false);
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		assert(false);
	}

	Mix_AllocateChannels(NUM_CHANNELS);

	for (auto pair = music_paths.begin(); pair != music_paths.end(); pair++) {
		AUDIO id = pair->first;
		auto& path = pair->second;
		Mix_Music* p = Mix_LoadMUS(path.c_str());
		if (p == nullptr) {
			fprintf(stderr, "Failed to open music file: ", path.c_str());
			assert(false);
		}
		music[id] = p;
	}

	for (auto pair = chunk_paths.begin(); pair != chunk_paths.end(); pair++) {
		AUDIO id = pair->first;
		auto& path = pair->second;
		Mix_Chunk* p = Mix_LoadWAV(path.c_str());
		if (p == nullptr) {
			fprintf(stderr, "Failed to open sound file: ", path.c_str());
			assert(false);
		}
		Mix_VolumeChunk(p, volumes[id]);
		chunks[id] = p;
	}

	// Playing background music indefinitely
	Mix_PlayMusic(music[MUSIC], -1);
	Mix_VolumeMusic(volumes[MUSIC]);
	fprintf(stderr, "Loaded music\n");
}