#pragma once
#include "Singleton.h"
#include <vector>
#include <memory>

class GameObject;

class CollisionManager :
  public Singleton<CollisionManager>
{
public:
  CollisionManager();
  ~CollisionManager();

  void Update(float a_fDeltaTime);

  void Register(std::weak_ptr<GameObject>);
  void Unregister(std::weak_ptr<GameObject>);

  std::vector<std::weak_ptr<GameObject>> gameObjects;
private:
  std::vector<std::weak_ptr<GameObject>> pendingUnregisters;
};
