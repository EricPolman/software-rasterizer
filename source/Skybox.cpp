#include "Skybox.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "Renderer.h"
#include "Triangle.h"
#include "MaterialManager.h"
#include "Camera.h"

Skybox::Skybox()
{
}


Skybox::~Skybox()
{
}

void Skybox::Render(const glm::mat4& matrix, bool filtered)
{
  const unsigned int numVerts = mesh->m_vertCount;
  for (unsigned int v = 0; v < numVerts; ++v)
  {
    mesh->m_vertices[v].rpos = vec3(vec4(mesh->m_vertices[v].pos, 1.0f) * matrix);
    mesh->m_vertices[v].rpos -= vec3(matrix[0][3], matrix[1][3], matrix[2][3]);
  }
  Materials->Get(mesh->m_material)->Apply();
  Renderer::DrawSkybox(this, filtered);
}