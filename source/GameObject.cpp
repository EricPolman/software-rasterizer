#include "GameObject.h"
#include "Triangle.h"
#include "Renderer.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "MaterialManager.h"
#include "CollisionManager.h"
#include "Camera.h"
#include "Asteroid.h"

GameObject::GameObject()
  : mesh(0), parent(nullptr), colliderRadius(0), type(UNDEFINED_OBJECT), destroy(false)
{

}


GameObject::~GameObject()
{
  for (auto i = children.begin(); i != children.end(); ++i)
  {
    CollisionManager::GetInstance()->Unregister(*i);
  }
  children.clear();
}


void GameObject::Update(float a_fDeltaTime)
{
  vec3 newPos = transform.GetPosition();
  bool changed = false;
  if (transform.GetPosition().x > 128)
  {
    newPos.x = 128;
    changed = true;
  }
  else if (transform.GetPosition().x < -128)
  {
    newPos.x = -128;
    changed = true;
  }

  if (transform.GetPosition().z > 128)
  {
    newPos.z = 128;
    changed = true;
  }
  else if (transform.GetPosition().z < -128)
  {
    newPos.z = -128;
    changed = true;
  }

  if (transform.GetPosition().y > 32)
  {
    newPos.y = 32;
    changed = true;
  }
  else if (transform.GetPosition().y < -32)
  {
    newPos.y = -32;
    changed = true;
  }
  if (changed)
  {
    transform.SetPosition(newPos);
    if (type == ASTEROID_OBJECT)
      ((Asteroid*)this)->RandomizeDirection();
  }

  transform.Update();
  for (auto i : pendingChildren)
  {
    children.push_back(i);
  }
  pendingChildren.clear();
  for (auto i = children.begin(); i != children.end();)
  {
    i->get()->Update(a_fDeltaTime);
    if (i->get()->destroy)
    {
      CollisionManager::GetInstance()->Unregister(*i);
      i = children.erase(i);
    }
    else
    {
      ++i;
    }
  }
}


void GameObject::Render(const glm::mat4& matrix)
{
  glm::mat4 transf = transform.GetMatrix() * matrix;
  transform.SetGlobalPosition((vec3(vec4(transf[0][3], transf[1][3], transf[2][3],1.0f) * Camera::g_transform)));

  if (mesh != 0)
  {
    Mesh* m = Meshes->Get(mesh);
    const unsigned int numVerts = m->m_vertCount;
    for (unsigned int v = 0; v < numVerts; ++v)
    {
      m->m_vertices[v].rpos = vec3(vec4(m->m_vertices[v].pos, 1.0f) * transf);
    }
    m->Draw();
  }

  for (auto i : children)
  {
    i->Render(transf);
  }
}
