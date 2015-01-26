#pragma once
#include "glm/glm.hpp"
#include "GameObject.h"

class Spaceship;

class Camera
  : public GameObject
{
public:
  Camera();
  ~Camera();

  static glm::mat4 g_transform;
  //virtual void Render(const glm::mat4& matrix);
  void CalculateMatrix();

  void Update(float a_fDeltaTime);
  Spaceship* player;
  glm::vec3 targetPosition;
private:
  glm::vec3 prevpos;
};

