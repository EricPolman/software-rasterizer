#pragma once
#include "Singleton.h"
#include <map>
#include <vector>

struct Mix_Chunk;
struct _Mix_Music;

class SoundManager : public Singleton<SoundManager>
{
  friend class Singleton<SoundManager>;
public:
	~SoundManager(void);
	SoundManager(void);

  std::vector<Mix_Chunk*> m_sounds;
  std::map<std::string, unsigned int> m_soundIds;
  unsigned int GetId(std::string name) { return m_soundIds[name]; }
  void Play(unsigned int id);
	unsigned int Add(std::string soundName);
	void Pause();
	void Resume();
	void SetMusic(std::string soundName, bool a_bStartPlaying);
private:
  _Mix_Music* m_pMusic;
};

#define Sounds SoundManager::GetInstance()
