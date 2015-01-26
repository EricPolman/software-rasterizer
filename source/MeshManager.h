#pragma once
#include "Singleton.h"
#include <map>
#include <vector>
#include <string>
#include "Mesh.h"

class MeshManager :
  public Singleton<MeshManager>
{
public:
  MeshManager(){}
  ~MeshManager(){ for (auto mesh : m_meshes) delete mesh; }

  unsigned int Add(const char* name, const char* path)
  {
    Mesh* t = new Mesh(path);
    const unsigned int id = m_meshes.size();
    m_meshes.push_back(t);
    m_nameToId[name] = id;
    return id;
  }
  Mesh* Get(const unsigned int idx) { return m_meshes[idx]; }
  unsigned int GetID(const char* name) { return m_nameToId[name]; }

  std::map<std::string, unsigned int> m_nameToId;
  std::vector<Mesh*> m_meshes;
};

#define Meshes MeshManager::GetInstance()

