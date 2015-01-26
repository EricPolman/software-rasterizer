#pragma once
#include "definitions.h"

class Triangle;
class Texture;
class Skybox;

class Renderer
{
public:
  static void Init();
  static void ClearZBuffer();
  static void DrawTriangle(Triangle* a_triangle);
  static void DrawSkybox(Skybox*, bool filtered = true);
  static unsigned long* g_screen;

  static Texture* g_currentColorMap;
  static unsigned int* g_currentColorBuffer;
  static unsigned int g_currentColorAverage;
  static bool g_textured;
  //static Texture* g_currentSpecMap;
  //static Texture* g_currentNormalMap;

  static unsigned int g_totalTriangles;
  static unsigned int g_trianglesDrawn;
  static unsigned int g_trianglesCulled;

};

