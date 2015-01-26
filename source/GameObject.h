#pragma once
#include <vector>
#include "glm/glm.hpp"
#include <memory>
#include "Transform.h"
#include "CollisionManager.h"

enum ObjectType
{
  UNDEFINED_OBJECT,
  PLAYER_OBJECT,
  BULLET_OBJECT,
  ASTEROID_OBJECT
};

class GameObject
{
public:
  GameObject();
  virtual ~GameObject();

  virtual void Update(float a_fDeltaTime);
  virtual void Render(const glm::mat4& matrix);
  void Destroy();

  unsigned int mesh;
  //unsigned int material;
  Transform transform;

  void AddChild(std::shared_ptr<GameObject> obj)
  {
    CollisionManager::GetInstance()->Register(obj);

    if (obj->parent)
      obj->parent->RemoveChild(obj);

    pendingChildren.push_back(obj);
    obj->parent = this;
  }
  void RemoveChild(std::shared_ptr<GameObject> obj)
  {
    auto objIt = std::find(children.begin(), children.end(), obj);
    if (objIt != children.end())
    {
      children.erase(objIt);
    }
  }
  GameObject* parent;
  std::vector<std::shared_ptr<GameObject>> children;
  std::vector<std::shared_ptr<GameObject>> pendingChildren;

  bool destroy;
  ObjectType type;
  float colliderRadius;
  virtual void OnCollision(GameObject* other) {}
};

