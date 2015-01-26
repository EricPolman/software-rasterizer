#pragma once
class Material
{
public:
  Material();
  ~Material();

  void Apply();
  static void Reset();

  unsigned int texture;
  unsigned int color;
  unsigned int ambientColor;
};

