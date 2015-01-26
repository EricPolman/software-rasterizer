#include "Camera.h"
#include "InputManager.h"
#include "Spaceship.h"
#include <stack>

using namespace glm;

glm::mat4 Camera::g_transform;

Camera::Camera()
{
  transform.SetPosition(vec3(0, -0.35f, -1.2f));
}


Camera::~Camera()
{
}

void Camera::Update(float a_fDeltaTime)
{
  GameObject::Update(a_fDeltaTime);
}

void Camera::CalculateMatrix()
{
  std::stack<GameObject*> parents;
  GameObject* par = parent;
  while (par != nullptr)
  {
    parents.push(par);
    par = par->parent;
  }

  glm::mat4 camMat = glm::mat4(1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f);
  while (!parents.empty())
  {
    camMat *= parents.top()->transform.GetMatrix();
    parents.pop();
  }

  g_transform = transform.GetMatrix() * camMat;
}
