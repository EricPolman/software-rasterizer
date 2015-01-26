#pragma once
#include "Singleton.h"
#include "glm/glm.hpp"

class LightManager : public Singleton<LightManager>
{
public:
  LightManager();
  ~LightManager();

  glm::vec3 lightDir;
  glm::vec3 lightDirTransformed;
  unsigned long lightColor;
  unsigned long ambientTerm;
  float diffuseTerm;
};

#define Lights LightManager::GetInstance()
