#include "Mesh.h"
#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>
#include "Triangle.h"
#include "Renderer.h"
#include <map>
#include "MaterialManager.h"
#include "TextureManager.h"

#define SCALE_FACTOR 0.1f

Mesh::Mesh(const char* a_path)
: m_transform(1.0f,0,0,0, // Identity matrix
              0,1.0f,0,0,
              0,0,1.0f,0,
              0, 0, 0, 1.0f),
  m_triCount(0), m_vertCount(0), m_material(0)
{
  Load(a_path);
}


Mesh::~Mesh(void)
{
  delete[] m_triangles;
  delete[] m_vertices;
  delete[] m_normals;
  delete[] m_uvcoords;
}

void Mesh::ParseMtl(const char* a_path)
{
  FILE* file = fopen(a_path, "r");
  if (!file)
    assert(0 && "Can't open file!");

  Material* mat = nullptr;

  while (1)
  {
    char lineHeader[128];

    int res = fscanf(file, "%s", lineHeader);
    if (res == EOF)
      break;

    if (strcmp(lineHeader, "newmtl") == 0)
    {
      char rest[128];
      fscanf(file, "%s", rest);
      std::string matName = a_path;
      matName.append(rest);
      unsigned int matId = Materials->Add(matName.c_str());
      mat = Materials->Get(matId);
    }
    else if (strcmp(lineHeader, "Kd") == 0)
    {
      // Diffuse
      float r, g, b;
      unsigned int ir, ig, ib;

      fscanf(file, "%f %f %f\n", &r, &g, &b);

      ir = (unsigned int)(255 * r);
      ig = (unsigned int)(255 * g);
      ib = (unsigned int)(255 * b);
      mat->color = (ir << 16) + (ig << 8) + ib;
      Textures->Get(0)->AddTint(mat->color);
    }
    else if (strcmp(lineHeader, "Ka") == 0)
    {
      // Diffuse
      float r, g, b;
      unsigned int ir, ig, ib;

      fscanf(file, "%f %f %f\n", &r, &g, &b);

      ir = (unsigned int)(255 * r);
      ig = (unsigned int)(255 * g);
      ib = (unsigned int)(255 * b);
      mat->ambientColor = (ir << 16) + (ig << 8) + ib;
    }
    else if (strcmp(lineHeader, "Ns") == 0)
    {
      // Specular
    }
    else if (strcmp(lineHeader, "map_Kd") == 0)
    {
      // Texture map
      char rest[128];
      fscanf(file, "%s", rest);

      std::string texPath = "resources/";
      texPath.append(rest);

      unsigned int texId = Textures->Add(rest, texPath.c_str());

      mat->texture = texId;
      mat->color = 0xFFFFFF;
    }
    else if (strcmp(lineHeader, "bump") == 0)
    {
      // Normal map
      /*char rest[128];
      fscanf(file, "%s", rest);

      Texture* tex = new Texture();
      mat->normalMap = tex;
      std::string texPath = "resources/";
      texPath.append(rest);
      tex->Load(texPath.c_str());
      printf(texPath.c_str());
      printf("\n%i\n", tex->height);*/
    }
  }

  fclose(file);
}

void Mesh::Load(const char* a_path)
{
  static std::map<std::string, int> idMap;
  idMap.clear();
  //Used http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/ 
  //for learning how to load an .OBJ model.
  std::string meshPath = a_path;
  meshPath.append(".obj");
  std::string mtlPath = a_path;
  mtlPath.append(".mtl");
  ParseMtl(mtlPath.c_str());
  FILE* file = fopen(meshPath.c_str(), "r");
  if (!file)
    assert(0 && "Can't open file!");

  std::vector<vec3> normals;
  std::vector<vec3> vertices;
  std::vector<vec2> uvcoords;

  std::vector<unsigned int> vertexIndices;
  std::vector<unsigned int> normalIndices;
  std::vector<unsigned int> uvcoordIndices;

  while (1)
  {
    char lineHeader[128];

    int res = fscanf(file, "%s", lineHeader);
    if (res == EOF)
      break;

    if (strcmp(lineHeader, "usemtl") == 0) //First character of line is "v", indicating vertex
    {
      char rest[128];
      fscanf(file, "%s", rest);
    
      std::string matName = mtlPath;
      matName.append(rest);
      m_material = Materials->GetID(matName.c_str());
    }
    else if (strcmp(lineHeader, "v") == 0) //First character of line is "v", indicating vertex
    {
      vec3 vertex;
      fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
      vertices.push_back(vertex * SCALE_FACTOR);
    }
    else if (strcmp(lineHeader, "vt") == 0)
    {
      vec2 uvCoord;
      fscanf(file, "%f %f\n", &uvCoord.x, &uvCoord.y);
      uvcoords.push_back(uvCoord);
    }
    else if (strcmp(lineHeader, "vn") == 0)
    {
      vec3 normal;
      fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
      normals.push_back(normal);
    }
    else if (strcmp(lineHeader, "f") == 0)
    {
      std::string vertex1, vertex2, vertex3;
      unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
      int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
        &vertexIndex[0], &uvIndex[0], &normalIndex[0],
        &vertexIndex[1], &uvIndex[1], &normalIndex[1],
        &vertexIndex[2], &uvIndex[2], &normalIndex[2]
        );

      if (matches != 9){
        assert(0 && "Error found in your file.");
      }

      vertexIndices.push_back(vertexIndex[0]-1);
      vertexIndices.push_back(vertexIndex[1]-1);
      vertexIndices.push_back(vertexIndex[2]-1);
      uvcoordIndices.push_back(uvIndex[0]-1);
      uvcoordIndices.push_back(uvIndex[1]-1);
      uvcoordIndices.push_back(uvIndex[2]-1);
      normalIndices.push_back(normalIndex[0]-1);
      normalIndices.push_back(normalIndex[1]-1);
      normalIndices.push_back(normalIndex[2]-1);
    }
  }
  fclose(file);

  m_vertices = new Vertex[vertices.size()];
  for (auto i : vertices)
  {
    Vertex vert(i.x, i.y, i.z);
    m_vertices[m_vertCount++] = vert;
  }
  m_normals = new vec3[normals.size()];
  m_uvcoords = new vec2[uvcoords.size()];
  std::copy(normals.begin(), normals.end(), m_normals);
  std::copy(uvcoords.begin(), uvcoords.end(), m_uvcoords);

  const unsigned int size = vertexIndices.size();
  m_triangles = new Triangle[size / 3];

  for (unsigned int i = 0; i < size; i += 3)
  {
    Triangle t(this,
      vertexIndices[i], vertexIndices[i + 1], vertexIndices[i + 2]);
    t.SetUVs(
      uvcoordIndices[i], uvcoordIndices[i + 1], uvcoordIndices[i + 2]);
    t.SetNormals(
      normalIndices[i], normalIndices[i + 1], normalIndices[i + 2]);

    m_triangles[m_triCount++] = t;
  }
}

void Mesh::Draw()
{
  Materials->Get(m_material)->Apply();
  for (unsigned int i = 0; i < m_triCount; ++i)
  {
    Renderer::DrawTriangle(&m_triangles[i]);
  }
  Materials->Get(m_material)->Reset();
}
