#pragma once
#include <vector>
#include "glm/glm.hpp"
#include <map>

using namespace glm;

class Triangle;
class Vertex;

class Mesh
{
public:
  Mesh(){}
  Mesh(const char* a_path);
  ~Mesh(void);

  void Draw();

  void ParseMtl(const char* a_path);
  void Load(const char* a_path);

  Triangle* m_triangles;
  Vertex* m_vertices;
  vec3* m_normals;
  vec2* m_uvcoords;

  mat4 m_transform;
  unsigned int m_triCount, m_vertCount;
  unsigned int m_material;
};