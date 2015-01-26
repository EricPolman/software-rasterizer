#include "Texture.h"
#include "FreeImage/FreeImage.h"

uint Texture::mortonX[MAX_TEXTURE_SIZE];
uint Texture::mortonY[MAX_TEXTURE_SIZE];

Texture::Texture()
{

}


Texture::~Texture()
{
  for (auto buffer : buffers)
    delete[] buffer.second;
}


void Texture::Load(const char* path)
{
  FREE_IMAGE_FORMAT fileType = FreeImage_GetFileType(path);
  FIBITMAP* temp = FreeImage_Load(fileType, path);

  width = FreeImage_GetWidth(temp);
  fwidth = (float)width;
  height = FreeImage_GetHeight(temp);
  fheight = (float)width;
  unsigned int powwidth = width;

  power = 0;
  while (powwidth >>= 1) ++power;

  buffers[0xFFFFFF] = new uint[height * width];

  unsigned long accumRed = 0;
  unsigned long accumBlue = 0;
  unsigned long accumGreen = 0;

  for (uint y = 0; y < height; ++y)
  {
    for (uint x = 0; x < width; ++x)
    {
      RGBQUAD col;
      FreeImage_GetPixelColor(temp, x, y, &col);
      uint color = (((uint)(col.rgbRed)) << 16) + (((uint)(col.rgbGreen) << 8)) + col.rgbBlue;
      buffers[0xFFFFFF][mortonX[x] | mortonY[y]] = color;
      accumRed += (color & 0xFF0000) >> 16;
      accumGreen += (color & 0xFF00) >> 8;
      accumBlue += (color & 0xFF);
    }
  }
  accumRed /= width * height;
  accumGreen /= width * height;
  accumBlue /= width * height;

  averagedColors[0xFFFFFF] = ((accumRed & 0xFF) << 16) + ((accumGreen & 0xFF) << 8) + (accumBlue & 0xFF);
  averagedColors[0] = ((accumRed & 0xFF) << 16) + ((accumGreen & 0xFF) << 8) + (accumBlue & 0xFF);

  FreeImage_Unload(temp);
}


void Texture::AddTint(uint tint)
{
  for (auto i : buffers)
    if (i.first == tint)
      return;

  buffers[tint] = new uint[height * width];
  unsigned long accumRed = 0;
  unsigned long accumBlue = 0;
  unsigned long accumGreen = 0;

  for (uint y = 0; y < height; ++y)
  {
    for (uint x = 0; x < width; ++x)
    {
      const uint col = buffers[0xFFFFFF][mortonX[x] | mortonY[y]];
      const uint r = (((tint & 0xFF0000) >> 16) * ((col & 0xFF0000) >> 16)) << 8;
      const uint g = (((tint & 0xFF00) >> 8) * ((col & 0xFF00) >> 8));
      const uint b = ((tint & 0xFF) * (col & 0xFF)) >> 8;

      buffers[tint][mortonX[x] | mortonY[y]] = (r & 0xFF0000) + (g & 0xFF00) + (b & 0xFF);
      accumRed += (r & 0xFF0000) >> 16;
      accumGreen += (g & 0xFF00) >> 8;
      accumBlue += (b & 0xFF);

    }
  }
  accumRed /= width * height;
  accumGreen /= width * height;
  accumBlue /= width * height;

  averagedColors[tint] = ((accumRed & 0xFF) << 16) + ((accumGreen & 0xFF) << 8) + (accumBlue & 0xFF);
}
