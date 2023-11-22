#pragma once
#include <unordered_map>
#include <SDL_mixer.h>
#include <common.hpp>


class AudioSystem {
public:
	enum AUDIO {
		MUSIC,
		MOB_HIT,
		MOB_DEATH,
		SHOT,
		SHOT_MG,
		SHOT_CROSSBOW,
	};

	const unsigned NUM_CHANNELS = 16;	// Total number of channels used for sound effects
	const unsigned MOB_HIT_CHANNEL = 0;	// Channel dedicated for MOB_HIT calls since it sounds super annoying
										// Ensures only 1 MOB_HIT can be played at the same time

	const float music_volume = 0.5f; // Range must be 0..1

private:
	/// <summary>
	/// Put background BGM here. DO NOT PUT SOUND EFFECTS.
	/// </summary>
	const std::unordered_map<AUDIO, std::string> music_paths {
		{ MUSIC, audio_path("music.wav") }
	};

	/// <summary>
	/// Put sound effects here. DO NOT PUT MUSIC.
	/// </summary>
	const std::unordered_map<AUDIO, std::string> chunk_paths {
		{MOB_DEATH, audio_path("mob_death1.wav")},
		{MOB_HIT, audio_path("mob_hit1.wav")},
		{SHOT, audio_path("shot1.wav")},
		{SHOT_MG, audio_path("shot_mg1.wav")},
		{SHOT_CROSSBOW, audio_path("shot_crossbow1.wav")},
	};

	/// <summary>
	/// Represents the volume of each sound effect (+ entire music channel)
	/// </summary>
	std::unordered_map<AUDIO, int> volumes {
		{MUSIC, MIX_MAX_VOLUME * music_volume},
		{MOB_DEATH, MIX_MAX_VOLUME * 0.5f},
		{MOB_HIT, MIX_MAX_VOLUME * 0.25f},
		{SHOT, MIX_MAX_VOLUME * 0.72f},
		{SHOT_MG, MIX_MAX_VOLUME * 0.55f},
		{SHOT_CROSSBOW, MIX_MAX_VOLUME * 0.75f},
	};

	std::unordered_map<AUDIO, Mix_Music*> music;	// Pointers containing music in memory
	std::unordered_map<AUDIO, Mix_Chunk*> chunks;	// Pointers containing sound effects in memory

	int current_random_channel = 0;	// counter for next_random_channel. Do not touch.

	/// <summary>
	/// Returns the channel number where the sound effect should be played.
	/// This ensures that sound effects do not prematurely end each other.
	/// </summary>
	/// <returns>Channel number</returns>
	int next_random_channel() {
		current_random_channel++;
		if (current_random_channel >= NUM_CHANNELS)
			current_random_channel = MOB_HIT_CHANNEL + 1;	// Skip the mob hurt channel
		Mix_HaltChannel(current_random_channel);
		return current_random_channel;
	}

public: 

	void init();

	~AudioSystem() {
		for (auto& m : music) {
			Mix_FreeMusic(m.second);
		}

		for (auto& m : chunks) {
			Mix_FreeChunk(m.second);
		}
		Mix_CloseAudio();
	}

	/// <summary>
	/// Plays a given sound. Supports instantaneous playment of music, but not recommended.
	/// </summary>
	/// <param name="id">Audio ID of the sound</param>
	void play_one_shot(AUDIO id) {
		bool is_music = music_paths.count(id);
		if (is_music)
			Mix_PlayMusic(music[id], 0);
		else {
			int channel = id == MOB_HIT ? MOB_HIT_CHANNEL : next_random_channel();	// Divert MOB_HIT calls to dedicated channel
			Mix_PlayChannel(channel, chunks[id], 0);
		}
			
	};

};