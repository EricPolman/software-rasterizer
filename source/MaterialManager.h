#pragma once
#include "Singleton.h"

#include <map>
#include <vector>
#include <string>
#include "Material.h"

class MaterialManager :
  public Singleton<MaterialManager>
{
public:
  MaterialManager(){}
  ~MaterialManager(){ for (auto mat : m_materials) delete mat; }

  unsigned int Add(const char* name)
  {
    Material* t = new Material();
    const unsigned int id = m_materials.size();
    m_materials.push_back(t);
    m_nameToId[name] = id;
    return id;
  }
  Material* Get(const unsigned int idx) { return m_materials[idx]; }
  unsigned int GetID(const char* name) { return m_nameToId[name]; }

  std::map<std::string, unsigned int> m_nameToId;
  std::vector<Material*> m_materials;
};

#define Materials MaterialManager::GetInstance()

