#pragma once
#include "GameObject.h"
class Bullet :
  public GameObject
{
public:
  Bullet(const glm::vec3& pos, const glm::vec3& dir);
  ~Bullet();

  float speed, distanceTravelled;

  glm::vec3 direction;

  void Update(float a_fDeltaTime);
  void OnCollision(GameObject* other);
};

