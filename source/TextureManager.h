#pragma once
#include "Singleton.h"
#include <map>
#include <vector>
#include <string>
#include "Texture.h"

class TextureManager :
  public Singleton<TextureManager>
{
public:
  TextureManager(){}
  ~TextureManager(){ for (auto tex : m_textures) delete tex; }

  unsigned int Add(const char* name, const char* path)
  {
    Texture* t = new Texture(); t->Load(path);
    const unsigned int id = m_textures.size();
    m_textures.push_back(t);
    m_nameToId[name] = id;
    return id;
  }
  Texture* Get(const unsigned int idx) { return m_textures[idx]; }
  unsigned int GetID(const char* name) { return m_nameToId[name]; }

  std::map<std::string, unsigned int> m_nameToId;
  std::vector<Texture*> m_textures;
};

#define Textures TextureManager::GetInstance()
