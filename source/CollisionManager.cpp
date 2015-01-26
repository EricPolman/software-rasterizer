#include "CollisionManager.h"
#include "GameObject.h"

CollisionManager::CollisionManager()
{
  gameObjects.reserve(512);
}


CollisionManager::~CollisionManager()
{
}

void CollisionManager::Update(float a_fDeltaTime)
{
  for (auto i : pendingUnregisters)
  {
    std::vector<std::weak_ptr<GameObject>>::iterator it = gameObjects.end();
    for (auto j = gameObjects.begin(); j != gameObjects.end(); ++j)
    {
      if (i.lock().get() == j->lock().get())
      {
        gameObjects.erase(j);
        break;
      }
    }
  }

  const unsigned int size = gameObjects.size();
  if (size < 1)
    return;

  for (auto a = gameObjects.begin(); a != gameObjects.end() - 1; ++a)
  {
    GameObject* aptr = a->lock().get();
    if (aptr == nullptr)
      continue;

    float aRadSq = aptr->colliderRadius * aptr->colliderRadius;
    const glm::vec3& aPos = aptr->transform.GetGlobalPosition();

    for (auto b = a + 1; b != gameObjects.end(); ++b)
    {
      GameObject* bptr = b->lock().get();
      if (bptr == nullptr)
        continue;

      float bRadSq = bptr->colliderRadius * bptr->colliderRadius;
      const glm::vec3& bPos = bptr->transform.GetGlobalPosition();

      const glm::vec3 diff = (bPos - aPos);
      float distSq = glm::dot(diff, diff);
      if (distSq <= aRadSq + bRadSq)
      {
        aptr->OnCollision(bptr);
        bptr->OnCollision(aptr);
      }
    }
  }
}

void CollisionManager::Register(std::weak_ptr<GameObject> a_ptr)
{
  bool exists = false;
  for (auto i : gameObjects)
    if (i.lock().get() == a_ptr.lock().get())
      exists = true;

  if (!exists)
    gameObjects.push_back(a_ptr);
}

void CollisionManager::Unregister(std::weak_ptr<GameObject> a_ptr)
{
  pendingUnregisters.push_back(a_ptr);
}
