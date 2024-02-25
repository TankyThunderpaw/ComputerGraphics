#include "sound.h"

// SDL2
#include "SDL.h"
#include "SDL_mixer.h"

#include <map>
#include <iostream>
#include <string>

// sound list

std::map<std::string, Mix_Music*> musicTracks;
std::map<std::string, Mix_Chunk*> soundEffects;


static void loadMusic(const char* id, const char* musicName);
static void loadSoundEffect(const char* id, const char* soundEffectName);

void initSounds(int channelNumbers)
{
	// Init SDL2 (we use it only to play sounds)
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        std::cout << "SDL could not be initialized!" << std::endl
            << "SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    // Init SDL2_mixer
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
    {
        std::cout << "SDL2_mixer could not be initialized!" << std::endl
            << "SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    Mix_AllocateChannels(channelNumbers);

    // Load tracks
    loadMusic("neuron", "resources/85046_newgrounds_parago.mp3");


    // Load sound effects
    loadSoundEffect("fire", "resources/fire.mp3");
    loadSoundEffect("hit", "resources/hit.mp3");
    loadSoundEffect("bounty", "resources/bounty.mp3");
    loadSoundEffect("bomb", "resources/bomb.mp3");

}

static void loadMusic(const char* id, const char* musicName)
{
    Mix_Music* music;
    
    if (!(music = Mix_LoadMUS(musicName)))
    {
        std::cout << "Music '" << musicName << "' could not be loaded!" << std::endl
            << "SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    musicTracks.insert({ id, music});
}

static void loadSoundEffect(const char* id, const char* soundEffectName)
{
    Mix_Chunk* sound;

    if (!(sound = Mix_LoadWAV(soundEffectName)))
    {
        std::cout << "Sound effect '" << soundEffectName << "' could not be loaded!" << std::endl
            << "SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    soundEffects.insert({ id, sound });
}

void playMusic(const char* id)
{
    if (musicTracks.count(id))
    {
        Mix_PlayMusic(musicTracks.at(id), -1);
    }
}

// Stops any music playing
void stopMusic()
{
    Mix_HaltMusic();
}

void playSoundEffect(int channel, const char* id)
{
    if (soundEffects.count(id))
    {
        Mix_PlayChannel(channel, soundEffects.at(id), 0);
    }
}