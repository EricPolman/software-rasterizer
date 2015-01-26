#pragma once
#include "glm/glm.hpp"
#include <map>

using namespace glm;
#define MAX_TEXTURE_SIZE 2048

class Texture
{
public:
  Texture();
  ~Texture();

  void Load(const char* path);
  void AddTint(uint tint);
  uint* GetBuffer(uint tint = 0xFFFFFF) { return buffers[tint]; }

  unsigned int width, height;
  float fwidth, fheight;
  unsigned int power;
  
  std::map<uint, uint*> buffers;
  std::map<uint, uint> averagedColors;


  // http://www.forceflow.be/2013/10/07/morton-encodingdecoding-through-bit-interleaving-implementations/
  // Suggested by Max Oomen
  static uint mortonX[MAX_TEXTURE_SIZE];
  static uint mortonY[MAX_TEXTURE_SIZE];

  static void InitMortonCurveLookupTable()
  {
    for (unsigned int x = 0; x < MAX_TEXTURE_SIZE; ++x)
    {
      unsigned int result = 0;

      for (unsigned int i = 0; i < 15; ++i)
        result |= ((x >> i) & 1) << (i * 2);
      mortonX[x] = result;
      mortonY[x] = result << 1;
    }
  }
};

