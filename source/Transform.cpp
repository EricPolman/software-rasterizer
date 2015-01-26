#include "Transform.h"

Transform::Transform() 
  : changed(false) 
{
  ComputeMatrix();
}


void Transform::ComputeMatrix()
{
  forward = glm::vec3(0, 0, 1) * orientation;
  right = glm::cross(forward, glm::vec3(0, 1, 0));
  up = glm::cross(forward, right);
  
  transform = glm::mat4_cast(orientation);
  transform[0][3] = position.x;
  transform[1][3] = position.y;
  transform[2][3] = position.z;

  //transform = glm::mat4(
  //  right.x, right.y, right.z, position.x,
  //  up.x, up.y, up.z, position.y,
  //  forward.x, forward.y, forward.z, position.z,
  //  0.0f, 0.0f, 0.0f, 1.0f
  //);

  changed = false;
}

void Transform::SetMatrix(glm::vec3& a_forward, glm::vec3& a_right, glm::vec3& a_up)
{
  forward = a_forward;
  right = glm::cross(forward, glm::vec3(0, 1, 0));
  up = glm::cross(forward, right);
  
  transform = glm::mat4(
    a_right.x, a_right.y, a_right.z, position.x,
    a_up.x, a_up.y, a_up.z, position.y,
    a_forward.x, a_forward.y, a_forward.z, position.z,
    0.0f, 0.0f, 0.0f, 1.0f
    );

  orientation = glm::quat_cast(transform);

  changed = false;
}