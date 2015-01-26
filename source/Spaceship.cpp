#include "Spaceship.h"
#include "InputManager.h"
#include "glm/gtx/euler_angles.hpp"
#include "Bullet.h"
#include "MeshManager.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "game.h"

using namespace glm;

Spaceship::Spaceship()
  : orientation(vec3()), yaw(0), pitch(0), roll(0), shootLeft(true)
{
  type = PLAYER_OBJECT;
  colliderRadius = 0.4f;
  invulnerable = false;
  invulnerableTimer = 0;
}


Spaceship::~Spaceship()
{
}

void Spaceship::Update(float a_fDeltaTime)
{
  if (invulnerable)
  {
    invulnerableTimer += a_fDeltaTime;
    if (invulnerableTimer > 1.0f)
    {
      invulnerable = false;
      invulnerableTimer = 0;
      Materials->Get(Meshes->Get(mesh)->m_material)->color = originalColor;
    }
  }
  else
  {
    if (Input->IsKeyDown(SDLK_a))
    {
      yaw += a_fDeltaTime * 2.0f;
    }
    if (Input->IsKeyDown(SDLK_d))
    {
      yaw -= a_fDeltaTime * 2.0f;
    }
    if (Input->IsKeyDown(SDLK_w))
    {
      if (pitch < M_PI / 4)
      {
        pitch += a_fDeltaTime * 2.0f;
      }
    }
    if (Input->IsKeyDown(SDLK_s))
    {
      if (pitch > -M_PI / 4)
        pitch -= a_fDeltaTime * 2.0f;
    }
    if (Input->IsKeyDown(SDLK_SPACE))
    {
      acceleration += transform.forward * a_fDeltaTime * 50.0f;
    }
    if (Input->IsKeyPressed(SDLK_LCTRL))
    {
      leftTurret = (vec3(-1.279f, 0.242f, 4.122f) * 0.1f) * transform.GetOrientation();
      rightTurret = (vec3(1.279f, 0.242f, 4.122f) * 0.1f) * transform.GetOrientation();

      parent->AddChild(std::shared_ptr<GameObject>(new Bullet(transform.GetPosition() + (shootLeft ? leftTurret : rightTurret), transform.forward)));
      shootLeft = !shootLeft;
    }
  }
  orientation = glm::quat_cast(glm::yawPitchRoll(0.0f, pitch, 0.0f)) * glm::quat_cast(glm::yawPitchRoll(yaw, 0.0f, 0.0f));
  
  velocity += acceleration * a_fDeltaTime;
  transform.Translate(velocity * a_fDeltaTime);
  transform.SetOrientation(orientation);

  velocity *= 0.98f;
  acceleration *= 0.98f;

  GameObject::Update(a_fDeltaTime);
}

void Spaceship::OnCollision(GameObject* other)
{
  if (other->type == ASTEROID_OBJECT)
  {
    if (!invulnerable)
    {
      const glm::vec3 direction = -normalize(other->transform.GetGlobalPosition() - transform.GetGlobalPosition());
      acceleration = vec3();
      velocity = glm::reflect(velocity, direction);
      invulnerable = true;
      --Tmpl8::Game::livesLeft;
      originalColor = Materials->Get(Meshes->Get(mesh)->m_material)->color;
      Materials->Get(Meshes->Get(mesh)->m_material)->color = 0xFF4444;
    }
  }
}