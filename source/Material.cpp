#include "Material.h"
#include "TextureManager.h"
#include "Renderer.h"

Material::Material()
  : texture(0), color(0xFFFFFF), ambientColor(0)
{
  
}


Material::~Material()
{
}

void Material::Apply()
{
  if (texture != 0) // no texture
  {
    Texture* tx = Textures->Get(texture);
    Renderer::g_currentColorMap = tx;
    Renderer::g_currentColorBuffer = tx->GetBuffer(color);
    Renderer::g_currentColorAverage = tx->averagedColors[color];
  }
  else
  {
    Texture* tx = Textures->Get(texture);
    Renderer::g_currentColorMap = tx;
    Renderer::g_currentColorBuffer = tx->GetBuffer(color);
    Renderer::g_textured = false;
    Renderer::g_currentColorAverage = color;
  }
}
void Material::Reset()
{
  Texture* tx = Textures->Get(0);
  Renderer::g_currentColorMap = tx;
  Renderer::g_textured = true;
}
