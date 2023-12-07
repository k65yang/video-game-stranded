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
		PLAYER_HIT,
		PLAYER_LOW_HEALTH,
		PLAYER_DEATH,
		SHOT,
		SHOT_MG,
		SHOT_CROSSBOW,
		SHOT_SHURIKEN,
		EMPTY_SHOTGUN,
		EMPTY_MG,
		EMPTY_CROSSBOW,
		RELOAD_SHOTGUN,
		RELOAD_MG,
		RELOAD_CROSSBOW,
		RELOAD_SHURIKEN,
		SHIP_ENTER,
		SHIP_LEAVE,
		CLICK,
		QUEST_PICKUP,
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
		{PLAYER_HIT, audio_path("player_hit.wav")},
		{PLAYER_LOW_HEALTH, audio_path("player_low_health.wav")},
		{PLAYER_DEATH, audio_path("player_death.wav")},
		{SHOT, audio_path("shot1.wav")},
		{SHOT_MG, audio_path("shot_mg1.wav")},
		{SHOT_CROSSBOW, audio_path("shot_crossbow1.wav")},
		{SHOT_SHURIKEN, audio_path("shot_shuriken.wav")}, 
		{EMPTY_SHOTGUN, audio_path("empty_shotgun.wav")},
		{EMPTY_MG, audio_path("empty_mg.wav")},
		{EMPTY_CROSSBOW, audio_path("empty_crossbow.wav")},
		{RELOAD_SHOTGUN, audio_path("reload_shotgun.wav")},
		{RELOAD_MG, audio_path("reload_mg.wav")},
		{RELOAD_CROSSBOW, audio_path("reload_crossbow.wav")},
		{RELOAD_SHURIKEN, audio_path("reload_shuriken.wav")},
		{SHIP_ENTER, audio_path("ship_enter.wav")},
		{SHIP_LEAVE, audio_path("ship_leave.wav")},
		{CLICK, audio_path("empty_crossbow.wav")},	// I actually think it's great for a clicking sound
		{QUEST_PICKUP, audio_path("quest_pickup.wav")}
	};

	/// <summary>
	/// Represents the volume of each sound effect (+ entire music channel)
	/// </summary>
	std::unordered_map<AUDIO, int> volumes {
		{MUSIC, MIX_MAX_VOLUME * music_volume},
		{MOB_DEATH, MIX_MAX_VOLUME * 0.5f},
		{MOB_HIT, MIX_MAX_VOLUME * 0.25f},
		{PLAYER_HIT, MIX_MAX_VOLUME * 0.55f},
		{PLAYER_LOW_HEALTH, MIX_MAX_VOLUME * 0.55f},
		{PLAYER_DEATH, MIX_MAX_VOLUME * 0.8f},
		{SHOT, MIX_MAX_VOLUME * 0.72f},
		{SHOT_MG, MIX_MAX_VOLUME * 0.55f},
		{SHOT_CROSSBOW, MIX_MAX_VOLUME * 0.75f},
		{SHOT_SHURIKEN, MIX_MAX_VOLUME * .6f},
		{EMPTY_SHOTGUN, MIX_MAX_VOLUME * .75f},
		{EMPTY_MG, MIX_MAX_VOLUME * 0.65f},
		{EMPTY_CROSSBOW, MIX_MAX_VOLUME * .65f},
		{RELOAD_SHOTGUN, MIX_MAX_VOLUME * .7f},
		{RELOAD_MG, MIX_MAX_VOLUME * 0.5f},
		{RELOAD_CROSSBOW, MIX_MAX_VOLUME * 0.65f},
		{RELOAD_SHURIKEN, MIX_MAX_VOLUME * 0.7f},
		{SHIP_ENTER, MIX_MAX_VOLUME * 0.7f},
		{SHIP_LEAVE, MIX_MAX_VOLUME * 0.7f},
		{QUEST_PICKUP, MIX_MAX_VOLUME * 0.5f},
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