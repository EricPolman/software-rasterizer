#pragma once
#include "glm/glm.hpp"

class Mesh;
class Texture;

class Skybox
{
public:
  Skybox();
  ~Skybox();

  Mesh* mesh;

  void Render(const glm::mat4& matrix, bool filtered = true);
};

