#include "Asteroid.h"
#include "MeshManager.h"
#include "game.h"
#include "SoundManager.h"

Asteroid::Asteroid(int a_size, glm::vec3 pos)
  : size(a_size), speed((float)(rand() % 256) / 256.0f * 16.0f)
{
  ++Tmpl8::Game::asteroids;
  transform.SetPosition(pos);
  RandomizeDirection();

  switch (size)
  {
  case 0:{ // Big
    mesh = Meshes->GetID("asteroid_big");
    colliderRadius = 3.8f;

    if (rand() % 4 == 0)
    {
      float rnd1 = 4.2f + (rand() % 256) / 128.0f;
      float rnd2 = 4.2f + (rand() % 256) / 128.0f;
      float rnd3 = 4.2f + (rand() % 256) / 128.0f;
      AddChild(std::shared_ptr<Asteroid>(new Asteroid(1, vec3(rnd1, rnd2, rnd3))));
    }
    break;
  }
  case 1:{
    mesh = Meshes->GetID("asteroid_medium");
    colliderRadius = 1.6f;

    if (rand() % 4 == 0)
    {
      float rnd1 = 2.0f + (rand() % 256) / 128.0f;
      float rnd2 = 2.0f + (rand() % 256) / 128.0f;
      float rnd3 = 2.0f + (rand() % 256) / 128.0f;
      AddChild(std::shared_ptr<Asteroid>(new Asteroid(2, vec3(rnd1, rnd2, rnd3))));
    }
    break;
  }
  case 2:
    mesh = Meshes->GetID("asteroid_small");
    colliderRadius = 0.7f;
    break;
  }
  
  type = ASTEROID_OBJECT;
}


Asteroid::~Asteroid()
{
  --Tmpl8::Game::asteroids;
}


void Asteroid::RandomizeDirection()
{
  glm::vec3 forward = normalize(vec3(rand() % 256 - 128, rand() % 256 - 128, rand() % 256 - 128));
  glm::vec3 right = normalize(cross(forward, vec3(0, 1, 0)));
  glm::vec3 up = cross(forward, right);
  transform.SetMatrix(forward, right, up);
}

void Asteroid::Update(float a_fDeltaTime)
{
  if (size == 0)
  {
    transform.Translate(transform.forward * a_fDeltaTime * speed);
    transform.Rotate(glm::quat(normalize(transform.forward + transform.up) * a_fDeltaTime * 0.5f));
  }
  else if(size == 1)
  {
    if (parent->type != ASTEROID_OBJECT)
      transform.Translate(transform.forward * a_fDeltaTime * 0.5f * speed);
    transform.Rotate(glm::quat(normalize(transform.forward) * a_fDeltaTime * 1.5f));
  }
  else
  {
    if (parent->type != ASTEROID_OBJECT)
    transform.Translate(transform.forward * a_fDeltaTime * 0.5f * speed);

    transform.Rotate(glm::quat(normalize(transform.forward) * a_fDeltaTime * 5.0f));
  }
  GameObject::Update(a_fDeltaTime);
} 


void Asteroid::OnCollision(GameObject* other)
{
  if (other->type == BULLET_OBJECT)
  {
    switch (size)
    {
    case 0:{ // Big
      Tmpl8::Game::score += 500;
      static std::vector<std::shared_ptr<GameObject>> childrenTransfer;
      for (auto i : children)
      {
        i->transform.SetPosition(i->transform.GetGlobalPosition() - parent->transform.GetGlobalPosition());
        i->transform.SetOrientation(i->transform.GetOrientation() * transform.GetOrientation());
        childrenTransfer.push_back(i);
      }
      for (auto i : childrenTransfer)
      {
        parent->AddChild(i);
      }
      for (uint i = 0; i < 2; ++i)
      {
        float rnd1 = 2.0f + (rand() % 256) / 64.0f - 2.0f;
        float rnd2 = 2.0f + (rand() % 256) / 64.0f - 2.0f;
        float rnd3 = 2.0f + (rand() % 256) / 64.0f - 2.0f;
        parent->AddChild(std::shared_ptr<Asteroid>(new Asteroid(1, transform.GetGlobalPosition() + vec3(rnd1, rnd2, rnd3))));
      }
      break;
    }
    case 1:{ 
      Tmpl8::Game::score += 250;
      static std::vector<std::shared_ptr<GameObject>> childrenTransfer;
      for (auto i : children)
      {
        i->transform.SetPosition(i->transform.GetGlobalPosition() - parent->transform.GetGlobalPosition());
        i->transform.SetOrientation(i->transform.GetOrientation() * transform.GetOrientation());
        childrenTransfer.push_back(i);
      }
      for (auto i : childrenTransfer)
      {
        parent->AddChild(i);
      }

      for (uint i = 0; i < 2; ++i)
      {
        float rnd1 = 2.0f + (rand() % 256) / 64.0f - 2.0f;
        float rnd2 = 2.0f + (rand() % 256) / 64.0f - 2.0f;
        float rnd3 = 2.0f + (rand() % 256) / 64.0f - 2.0f;
        parent->AddChild(std::shared_ptr<Asteroid>(new Asteroid(2, transform.GetGlobalPosition() - parent->transform.GetGlobalPosition() + vec3(rnd1, rnd2, rnd3))));
      }
      break;
    }
    case 2:
      Tmpl8::Game::score += 100;
      break;
    }
    Sounds->Play(1);
    destroy = true;
  }
}
