#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

class Transform
{
public:
  Transform();
  ~Transform(){}

  void Translate(glm::vec3& translation) { position += translation; changed = true; }
  void Rotate(glm::quat& rotation) { orientation = glm::normalize(orientation * rotation); changed = true; }

  void SetPosition(const glm::vec3& pos) { position = pos; changed = true; }
  void SetOrientation(glm::quat& rot) { orientation = rot; changed = true; }
  const glm::vec3& GetPosition() { return position; }
  const glm::quat& GetOrientation() { return orientation; }

  void Update() { if (changed) ComputeMatrix(); }

  const glm::mat4& GetMatrix() { return transform; }
  void SetMatrix(glm::vec3& a_forward, glm::vec3& a_right, glm::vec3& a_up);

  const glm::vec3& GetGlobalPosition() { return globalPosition; }
  void SetGlobalPosition(const glm::vec3& pos) { globalPosition = pos;  }

  glm::vec3 forward, right, up;
private:
  void ComputeMatrix();

  glm::vec3 position;
  glm::vec3 globalPosition;
  glm::quat orientation;
  glm::mat4 transform;
  bool changed;
};

