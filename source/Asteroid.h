#pragma once
#include "GameObject.h"
class Asteroid :
  public GameObject
{
public:
  Asteroid(int size, glm::vec3 pos);
  ~Asteroid();

  virtual void Update(float a_fDeltaTime);
  virtual void OnCollision(GameObject* other);
  int size;
  float speed;
  void RandomizeDirection();
};

