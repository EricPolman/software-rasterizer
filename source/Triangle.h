#pragma once
#include "glm/glm.hpp"
#include "Mesh.h"

using namespace glm;

class Vertex
{ 
public:
  Vertex(float a_x = 0.0f, float a_y = 0.0f, float a_z = 0.0f) : pos(a_x, a_y, a_z){}; 

  vec3 pos, rpos;
};

class Triangle
{
public:
  Triangle(){}
  Triangle(Mesh* a_mesh, unsigned int a_v0, unsigned int a_v1, unsigned int a_v2)
    : m_mesh(a_mesh), v0(a_v0), v1(a_v1), v2(a_v2) 
  {
  }

  Mesh* m_mesh;

  void SetUVs(unsigned int a_uv0, unsigned int a_uv1, unsigned int a_uv2)
  {
    uv0 = a_uv0;
    uv1 = a_uv1;
    uv2 = a_uv2;
  }
  void SetNormals(unsigned int a_n0, unsigned int a_n1, unsigned int a_n2)
  {
    n0 = a_n0;
    n1 = a_n1;
    n2 = a_n2;
  }

  Vertex* GetVertex(unsigned int n) const { return &m_mesh->m_vertices[n]; }
  vec2* GetUV(unsigned int n) const { return &m_mesh->m_uvcoords[n]; }
  vec3* GetNormal(unsigned int n) const { return &m_mesh->m_normals[n]; }

  unsigned int v0, v1, v2;
  unsigned int uv0, uv1, uv2;
  unsigned int n0, n1, n2;
};

