#pragma once
#include "GameObject.h"
#include "glm/gtc/quaternion.hpp"
#include "glm/glm.hpp"

class Spaceship :
  public GameObject
{
public:
  Spaceship();
  virtual ~Spaceship();

  virtual void Update(float a_fDeltaTime);

  glm::quat orientation;
  float yaw, pitch, roll;

  glm::vec3 acceleration, velocity;

  glm::vec3 leftTurret, rightTurret;
  bool shootLeft;

  void OnCollision(GameObject* other);

  float invulnerableTimer;
  bool invulnerable;
  unsigned int originalColor;
};

