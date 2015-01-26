#include "SoundManager.h"
#include <string>
#include "SDL.h"
#include "SDL_mixer.h"

SoundManager::SoundManager(void)
  : m_pMusic(nullptr)
{
	Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 1, 4096 );
}


SoundManager::~SoundManager(void)
{
	for(auto i : m_sounds)
  {
    Mix_FreeChunk(i);
  }
  if (m_pMusic != nullptr)
    Mix_FreeMusic(m_pMusic);
  Mix_CloseAudio();
}


void SoundManager::Play(unsigned int id)
{
  Mix_PlayChannel( -1, m_sounds[id], 0 );
}


void SoundManager::SetMusic(std::string a_soundName, bool a_bStartPlaying)
{
	m_pMusic = Mix_LoadMUS(std::string("resources/"+a_soundName+".wav").c_str());
	if(a_bStartPlaying)
		Mix_PlayMusic( m_pMusic, -1 );
}


unsigned int SoundManager::Add(std::string a_soundName)
{
  m_soundIds[a_soundName] = m_sounds.size();
	m_sounds.push_back(Mix_LoadWAV(std::string("resources/"+a_soundName+".wav").c_str()));
  return m_soundIds[a_soundName];
}


void SoundManager::Pause()
{
	Mix_PauseMusic();
}

void SoundManager::Resume()
{
	Mix_ResumeMusic();
}