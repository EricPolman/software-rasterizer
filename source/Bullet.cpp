#include "Bullet.h"
#include "MeshManager.h"
#include "SoundManager.h"

Bullet::Bullet(const glm::vec3& pos, const glm::vec3& dir)
  : direction(dir), speed(50.0f)
{
  mesh = Meshes->GetID("bullet");
  transform.SetPosition(pos);
  vec3 fwd = dir;
  vec3 right = normalize(cross(fwd, vec3(0, 1, 0)));
  vec3 up = cross(fwd, right);
  transform.SetMatrix(fwd, right, up);

  type = BULLET_OBJECT;
  colliderRadius = 0.05f;

  Sounds->Play(0);
}


Bullet::~Bullet()
{
}

void Bullet::Update(float a_fDeltaTime)
{
  transform.Translate(direction * speed * a_fDeltaTime);
  distanceTravelled += speed * a_fDeltaTime;
  if (distanceTravelled > 150.0f)
  {
    destroy = true;
  }
  GameObject::Update(a_fDeltaTime);
}


void Bullet::OnCollision(GameObject* other)
{
  if (other->type != PLAYER_OBJECT) 
    destroy = true;
}