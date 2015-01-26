#include "Renderer.h"
#include "Triangle.h"
#include "surface.h"
#include "TextureManager.h"
#include "LightManager.h"
#include "Skybox.h"

unsigned long* Renderer::g_screen;
Texture* Renderer::g_currentColorMap;
uint* Renderer::g_currentColorBuffer;
uint Renderer::g_currentColorAverage;
uint Renderer::g_totalTriangles = 0;
uint Renderer::g_trianglesDrawn = 0;
uint Renderer::g_trianglesCulled = 0;
bool Renderer::g_textured = true;
static float xmin[SCRHEIGHT], xmax[SCRHEIGHT], zmin[SCRHEIGHT], zmax[SCRHEIGHT];
static float umin[SCRHEIGHT], umax[SCRHEIGHT], vmin[SCRHEIGHT], vmax[SCRHEIGHT];

const static uint XMIN_IDX = 0, ZMIN_IDX = 1, UMIN_IDX = 2, VMIN_IDX = 3,
XMAX_IDX = 4, ZMAX_IDX = 5,UMAX_IDX = 6, VMAX_IDX = 7, MINMAXEL = 8;

static float zbuffer[SCRHEIGHT * SCRWIDTH];
static float minsmaxs[SCRHEIGHT * MINMAXEL];

static unsigned int bilinearFilteringPrecalcs[256][4];

struct ClippingPlane
{
  ClippingPlane() : normal(0, 0, 0), offset(0){}
  ClippingPlane(const vec3& n, float o) : normal(n), offset(o){}
  ClippingPlane(const vec3& n, const vec3& apos) : normal(n), pos(apos){}
  vec3 normal;
  vec3 pos;
  float offset;
};

Vertex clipBufferVertex[16];
uint clipBufferCounter = 0;

vec2 clipBufferUV1[9];
vec2 clipBufferUV2[9];
ClippingPlane clippingPlanes[6];
uint planeCount = 0;

Vertex* clippedVerts1[9];
Vertex* clippedVerts2[9];
// TODO: Add pointers to UVs here

Vertex** curSourceList, **curDestList;
vec2* curUVSourceList, *curUVDestList;
uint currentSourceCount = 0;
uint currentDestCount = 0;

vec2 screenCoords[9];

void Renderer::Init()
{
  for (int i = 0; i < SCRHEIGHT * MINMAXEL; i += MINMAXEL)
  {
    minsmaxs[i + XMIN_IDX] = SCRWIDTH - 1;
    minsmaxs[i + XMAX_IDX] = 0;
    minsmaxs[i + ZMIN_IDX] = 0;
    minsmaxs[i + ZMAX_IDX] = 0;
  }
  memset(clippedVerts1, 0, sizeof(clippedVerts1));
  memset(clippedVerts2, 0, sizeof(clippedVerts2));

  vec3 topLeftNear(-0.499f / ASPECT_RATIO, -0.499f, 1.0f);
  vec3 botLeftFar(-0.499f * 1000.0f / ASPECT_RATIO, 0.499f * 1000.0f, 1000.0f);
  vec3 topLeftFar(-0.499f * 1000.0f / ASPECT_RATIO, -0.499f * 1000.0f, 1000.0f);

  vec3 topRightNear(0.499f / ASPECT_RATIO, -0.499f, 1.0f);
  vec3 botRightFar(0.499f * 1000.0f / ASPECT_RATIO, 0.499f * 1000.0f, 1000.0f);
  vec3 topRightFar(0.499f * 1000.0f / ASPECT_RATIO, -0.499f * 1000.0f, 1000.0f);

  vec3 leftNormal = normalize(cross(botLeftFar - topLeftNear, topLeftFar - topLeftNear));
  vec3 rightNormal = normalize(cross(topRightFar - topRightNear, botRightFar - topRightNear));
  vec3 topNormal = normalize(cross(topLeftFar - topLeftNear, topRightFar - topLeftNear));

  clippingPlanes[planeCount++] = ClippingPlane(vec3(0, 0, 1), vec3(0, 0, 0.01f)); // Near clipping plane
  clippingPlanes[planeCount++] = ClippingPlane(leftNormal, topLeftNear); // Left clipping plane
  clippingPlanes[planeCount++] = ClippingPlane(rightNormal, topRightNear); // Right clipping plane

  for (unsigned int u = 0; u < 16; ++u)
  {
    for (unsigned int v = 0; v < 16; ++v)
    {
      bilinearFilteringPrecalcs[u + (v << 4)][0] = (unsigned int)((1.0f - ((float)u / 16.0f)) * (1.0f - ((float)v / 16.0f)) * 256.0f);
      bilinearFilteringPrecalcs[u + (v << 4)][1] = (unsigned int)(((float)u / 16.0f) *(1.0f - ((float)v / 16.0f)) * 256.0f);
      bilinearFilteringPrecalcs[u + (v << 4)][2] = (unsigned int)((1.0f - ((float)u / 16.0f)) * ((float)v / 16.0f) * 256.0f);
      bilinearFilteringPrecalcs[u + (v << 4)][3] = 256
        - bilinearFilteringPrecalcs[u + (v << 4)][0]
        - bilinearFilteringPrecalcs[u + (v << 4)][1]
        - bilinearFilteringPrecalcs[u + (v << 4)][2];
    }
  }                                                     
}


void Renderer::ClearZBuffer()
{
  memset(zbuffer, 0, sizeof(zbuffer));
}


void Renderer::DrawTriangle(Triangle* a_triangle)
{
  ++g_totalTriangles;

  clippedVerts1[0] = &a_triangle->m_mesh->m_vertices[a_triangle->v0];
  clippedVerts1[1] = &a_triangle->m_mesh->m_vertices[a_triangle->v1];
  clippedVerts1[2] = &a_triangle->m_mesh->m_vertices[a_triangle->v2];

  vec3 normal = glm::cross(clippedVerts1[2]->rpos - clippedVerts1[0]->rpos, clippedVerts1[1]->rpos - clippedVerts1[0]->rpos);
  if (glm::dot(normal, clippedVerts1[0]->rpos) <= 0)
  {
    ++g_trianglesCulled; return;
  }// Backface culling

  vec2* uvs[3] = { a_triangle->GetUV(a_triangle->uv0), a_triangle->GetUV(a_triangle->uv1), a_triangle->GetUV(a_triangle->uv2) };

  currentSourceCount = 3, currentDestCount = 3;
  curSourceList = clippedVerts2, curDestList = clippedVerts1;
  curUVSourceList = clipBufferUV1, curUVDestList = clipBufferUV2;

  curUVDestList[0] = *uvs[0], curUVDestList[1] = *uvs[1], curUVDestList[2] = *uvs[2];

  for (uint i = 0; i < planeCount; ++i) // Plane
  { // Swap pointers 
    Vertex** tempSrcList = curSourceList;
    curSourceList = curDestList, curDestList = tempSrcList;

    vec2* tempUVSrcList = curUVSourceList;
    curUVSourceList = curUVDestList, curUVDestList = tempUVSrcList;

    currentSourceCount = currentDestCount, currentDestCount = 0; // Swap counts

    for (uint e = 0; e < currentSourceCount; ++e) // Edge
    {
      Vertex* p0 = curSourceList[e];
      const uint p1id = (e + 1) % currentSourceCount;
      Vertex* p1 = curSourceList[p1id];

      bool firstIn = false, secondIn = false;
      vec3 planePos = clippingPlanes[i].pos;
      float p0dot = dot(clippingPlanes[i].normal, p0->rpos - planePos);
      float p1dot = dot(clippingPlanes[i].normal, p1->rpos - planePos);
      if (p0dot >= 0) firstIn = true; if (p1dot >= 0) secondIn = true;

      if (firstIn && secondIn) // Staying in
      {
        // Emit 2nd
        curUVDestList[currentDestCount] = curUVSourceList[p1id];
        curDestList[currentDestCount++] = p1;
      }
      else if (firstIn && !secondIn) // Going out
      {
        float d = dot(planePos, clippingPlanes[i].normal);
        float dist0 = dot(clippingPlanes[i].normal, p0->rpos) - d;
        float dist1 = dot(clippingPlanes[i].normal, p1->rpos) - d;
        float factor = dist0 / (fabsf(dist0) + fabsf(dist1));
        vec3 intersectionPoint(p0->rpos + factor * (p1->rpos - p0->rpos));

        Vertex* intersectionVert = &clipBufferVertex[clipBufferCounter++];
        intersectionVert->rpos = intersectionPoint;
        curUVDestList[currentDestCount] = curUVSourceList[e] + (factor * (curUVSourceList[p1id] - curUVSourceList[e]));

        curDestList[currentDestCount++] = intersectionVert;
      }
      else if (!firstIn && secondIn) // Going in
      {
        float d = dot(planePos, clippingPlanes[i].normal);
        float dist0 = dot(clippingPlanes[i].normal, p0->rpos) - d;
        float dist1 = dot(clippingPlanes[i].normal, p1->rpos) - d;
        float factor = dist1 / (fabsf(dist0) + fabsf(dist1));
        vec3 intersectionPoint(p1->rpos + factor * (p0->rpos - p1->rpos));

        Vertex* intersectionVert = &clipBufferVertex[clipBufferCounter++];
        intersectionVert->rpos = intersectionPoint;
        curUVDestList[currentDestCount] = curUVSourceList[p1id] + (factor * (curUVSourceList[e] - curUVSourceList[p1id]));
        curDestList[currentDestCount++] = intersectionVert;

        curUVDestList[currentDestCount] = curUVSourceList[p1id];
        curDestList[currentDestCount++] = p1;
      }
    }
  }
  clipBufferCounter = 0;

  if (currentDestCount == 0)
  {
    ++g_trianglesCulled;
    return;
  }
  ++g_trianglesDrawn;

  int startY = SCRHEIGHT - 1, endY = 0, minX = SCRWIDTH - 1, maxX = 0;

  const static float  scrwAddition = (SCRWIDTH / 10.0f) * ASPECT_RATIO;
  const static float  scrhAddition = (SCRHEIGHT / 10.0f);
  // Setup screen coords
  for (unsigned int i = 0; i < currentDestCount; ++i)
  {
    screenCoords[i] = vec2(curDestList[i]->rpos);

    screenCoords[i].x =
      screenCoords[i].x / (curDestList[i]->rpos.z * 0.1f)
      * scrwAddition + HALF_SCRW;

    screenCoords[i].y =
      screenCoords[i].y / (curDestList[i]->rpos.z * 0.1f)
      * scrhAddition + HALF_SCRH;
  }


  for (uint i = 0; i < currentDestCount; ++i)
  {
    Vertex* v0 = curDestList[i];
    uint v0id = i, v1id = (i + 1) % currentDestCount;
    Vertex* v1 = curDestList[v1id];

    if (screenCoords[v1id].y < screenCoords[v0id].y)
    { // Swap
      Vertex* temp = v0; v0 = v1, v1 = temp;
      uint uiTemp = v0id; v0id = v1id, v1id = uiTemp;
    }
    // Setup pixel vars
    const float y1 = screenCoords[v0id].y, y2 = screenCoords[v1id].y;

    if (y2 < 0 || (y2 == y1) || y1 > SCRHEIGHT - 1) continue;

    int iy1 = (int)(y1 + 1.0f), iy2 = (int)y2; // sub pix corr on iy1
    iy1 = iy1 > 0 ? iy1 : 0;

    // Setup additional vars
    float x1 = screenCoords[v0id].x, z1 = 1.0f / (v0->rpos.z);
    float UVu1 = curUVDestList[v0id].x * z1, UVv1 = curUVDestList[v0id].y * z1;

    const float x2 = screenCoords[v1id].x, z2 = 1.0f / (v1->rpos.z);
    const float UVu2 = curUVDestList[v1id].x * z2, UVv2 = curUVDestList[v1id].y * z2;

    iy2 = iy2 > SCRHEIGHT - 1 ? SCRHEIGHT - 1 : iy2;
    startY = iy1 < startY ? iy1 : startY, endY = iy2 > endY ? iy2 : endY;

    // Lerp values
    const float yDiff = 1.0f / (y2 - y1);
    const float dx = (x2 - x1) * yDiff, dz = (z2 - z1) * yDiff;
    const float du = (UVu2 - UVu1) * yDiff, dv = (UVv2 - UVv1) * yDiff;

    // Sub-pixel corrections
    const float sPC = ((float)iy1 - y1);
    x1 += dx * sPC, z1 += dz * sPC, UVu1 += du * sPC, UVv1 += dv * sPC;

    // Fill outline tables 
    float* pMM = &(minsmaxs[iy1 * MINMAXEL]);
    for (int y = iy1; y <= iy2; ++y)
    {
      if (x1 < pMM[XMIN_IDX])
      {
        pMM[XMIN_IDX] = x1, pMM[ZMIN_IDX] = z1, pMM[UMIN_IDX] = UVu1, pMM[VMIN_IDX] = UVv1;
        minX = minX > x1 ? x1 : minX;
      }

      if (x1 > pMM[XMAX_IDX])
      {
        pMM[XMAX_IDX] = x1, pMM[ZMAX_IDX] = z1, pMM[UMAX_IDX] = UVu1, pMM[VMAX_IDX] = UVv1;
        maxX = maxX < x1 ? x1 : maxX;
      }
      pMM += MINMAXEL, x1 += dx, z1 += dz, UVu1 += du, UVv1 += dv;
    }
  }

  // Calculate dimensions
  const int size = (maxX - minX) * (endY - startY);
  const bool usePerspCorr = size > 2500;
  // > 2500 = persp. corr

  // Calculate lighting term.
  normal = glm::normalize(normal);
  const int iLightTerm = (int)(((normal.x * Lights->lightDirTransformed.x) + (normal.y * Lights->lightDirTransformed.y) + (normal.z * Lights->lightDirTransformed.z)) * 255.0f * Lights->diffuseTerm);
  uint lightTerm = iLightTerm < 0 ? 0 : (uint)iLightTerm;
  lightTerm += Lights->ambientTerm;
  lightTerm = lightTerm > 255 ? 255 : lightTerm;

  const uint txwmin1 = (g_currentColorMap->width - 1);
  const uint txhmin1 = (g_currentColorMap->height - 1);
  // Draw spans
  float* pMM = &minsmaxs[startY * MINMAXEL];

  if (usePerspCorr)
  {
    for (int y = startY; y <= endY; ++y, pMM += MINMAXEL)
    {
      int ix1 = (int)(pMM[XMIN_IDX] + 1.0f), ix2 = (int)(pMM[XMAX_IDX]);

      const float xDiff = 1.0f / (pMM[XMAX_IDX] - pMM[XMIN_IDX]);

      float z1 = pMM[ZMIN_IDX], u1 = pMM[UMIN_IDX], v1 = pMM[VMIN_IDX];
      const float z2 = pMM[ZMAX_IDX], u2 = pMM[UMAX_IDX], v2 = pMM[VMAX_IDX];
      const float dz = (z2 - z1) * xDiff, du = (u2 - u1) * xDiff, dv = (v2 - v1) * xDiff;

      // Sub-texel correction on UV values
      const float subTexCorr = ((float)ix1 - pMM[XMIN_IDX]);
      z1 += dz * subTexCorr, v1 += dv * subTexCorr, u1 += du * subTexCorr;
      ix2 = ix2 > SCRWIDTH - 1 ? SCRWIDTH - 1 : ix2;

      unsigned long* screenBuffer = &g_screen[y*SCRWIDTH + ix1];
      float* zBuffer = &zbuffer[y * SCRWIDTH + ix1];

      for (int x = ix1; x <= ix2; ++x)
      {
        if (z1 > *zBuffer)
        {
          const float recipZ = 1.0f / z1; // is 1/1/Z

          const uint uCoord = (uint)(u1 * recipZ * g_currentColorMap->fwidth) & txwmin1;
          const uint vCoord = (uint)(v1 * recipZ * g_currentColorMap->fheight) & txhmin1;

          const uint col = g_currentColorBuffer[Texture::mortonX[uCoord] | Texture::mortonY[vCoord]];

          const unsigned int rb = (((col & REDBLUEMASK) * lightTerm) >> 8) & REDBLUEMASK;
          const unsigned int g = (((col & GREENMASK) * lightTerm) >> 8) & GREENMASK;

          *screenBuffer = rb + g;
          *zBuffer = z1;
        }
        ++zBuffer, ++screenBuffer, z1 += dz, u1 += du, v1 += dv;
      }
      pMM[XMIN_IDX] = SCRWIDTH - 1, pMM[XMAX_IDX] = 0;
    }
  }
  else
  {
    for (int y = startY; y <= endY; ++y, pMM += MINMAXEL)
    {
      int ix1 = (int)(pMM[XMIN_IDX] + 1.0f), ix2 = (int)(pMM[XMAX_IDX]);

      const float xDiff = 1.0f / (pMM[XMAX_IDX] - pMM[XMIN_IDX]);
      float z1 = pMM[ZMIN_IDX], u1 = pMM[UMIN_IDX] / z1, v1 = pMM[VMIN_IDX] / z1;
      const float z2 = pMM[ZMAX_IDX], u2 = pMM[UMAX_IDX] / z2, v2 = pMM[VMAX_IDX] / z2;
      const float dz = (z2 - z1) * xDiff, du = (u2 - u1) * xDiff, dv = (v2 - v1) * xDiff;

      // Sub-texel correction on UV values
      const float subTexCorr = ((float)ix1 - pMM[XMIN_IDX]);
      z1 += dz * subTexCorr, v1 += dv * subTexCorr, u1 += du * subTexCorr;

      unsigned long* screenBuffer = &g_screen[y*SCRWIDTH + ix1];
      float* zBuffer = &zbuffer[y * SCRWIDTH + ix1];
      if (!g_textured)
      {
        for (int x = ix1; x <= ix2; ++x)
        {
          if (z1 > *zBuffer)
          {
            const uint col = g_currentColorAverage;

            const unsigned int rb = (((col & REDBLUEMASK) * lightTerm) >> 8) & REDBLUEMASK;
            const unsigned int g = (((col & GREENMASK) * lightTerm) >> 8) & GREENMASK;

            *screenBuffer = rb + g;
            *zBuffer = z1;
          }
          ++zBuffer, ++screenBuffer, z1 += dz, u1 += du, v1 += dv;
        }
      }
      else
      {
        for (int x = ix1; x <= ix2; ++x)
        {
          if (z1 > *zBuffer)
          {
            const uint uCoord = (uint)(u1 * g_currentColorMap->fwidth) & txwmin1;
            const uint vCoord = (uint)(v1 * g_currentColorMap->fheight) & txhmin1;

            const uint col = g_currentColorBuffer[Texture::mortonX[uCoord] | Texture::mortonY[vCoord]];

            const unsigned int rb = (((col & REDBLUEMASK) * lightTerm) >> 8) & REDBLUEMASK;
            const unsigned int g = (((col & GREENMASK) * lightTerm) >> 8) & GREENMASK;

            *screenBuffer = rb + g;
            *zBuffer = z1;
          }
          ++zBuffer, ++screenBuffer, z1 += dz, u1 += du, v1 += dv;
        }
      }
      pMM[XMIN_IDX] = SCRWIDTH - 1, pMM[XMAX_IDX] = 0;
    }
  }
}

void Renderer::DrawSkybox(Skybox* skybox, bool filtered)
{
  Triangle* tris = skybox->mesh->m_triangles;
  unsigned int triCount = skybox->mesh->m_triCount;

  for (uint t = 0; t < triCount; ++t)
  {
    ++g_totalTriangles;

    Vertex* verts[3] = {
      tris[t].GetVertex(tris[t].v0),
      tris[t].GetVertex(tris[t].v1),
      tris[t].GetVertex(tris[t].v2) };

    vec2* uvs[3] = {
      tris[t].GetUV(tris[t].uv0),
      tris[t].GetUV(tris[t].uv1),
      tris[t].GetUV(tris[t].uv2) };

    memcpy(clippedVerts1, verts, sizeof(Vertex*) * 3); // Copy verts 

    currentSourceCount = 3, currentDestCount = 3;
    curSourceList = clippedVerts2, curDestList = clippedVerts1;
    curUVSourceList = clipBufferUV1, curUVDestList = clipBufferUV2;

    curUVDestList[0] = *uvs[0], curUVDestList[1] = *uvs[1], curUVDestList[2] = *uvs[2];

    for (uint i = 0; i < planeCount; ++i) // Plane
    { // Swap pointers 
      Vertex** tempSrcList = curSourceList;
      curSourceList = curDestList, curDestList = tempSrcList;

      vec2* tempUVSrcList = curUVSourceList;
      curUVSourceList = curUVDestList, curUVDestList = tempUVSrcList;

      currentSourceCount = currentDestCount, currentDestCount = 0; // Swap counts

      for (uint e = 0; e < currentSourceCount; ++e) // Edge
      {
        Vertex* p0 = curSourceList[e];
        const uint p1id = (e + 1) % currentSourceCount;
        Vertex* p1 = curSourceList[p1id];

        bool firstIn = false, secondIn = false;
        vec3 planePos = clippingPlanes[i].pos;
        float p0dot = dot(clippingPlanes[i].normal, p0->rpos - planePos);
        float p1dot = dot(clippingPlanes[i].normal, p1->rpos - planePos);
        if (p0dot >= 0)
          firstIn = true;
        if (p1dot >= 0)
          secondIn = true;

        if (firstIn && secondIn) // Staying in
        { // Emit 2nd
          curUVDestList[currentDestCount] = curUVSourceList[p1id];
          curDestList[currentDestCount++] = p1;
        }
        else if (firstIn && !secondIn) // Going out
        {
          float d = dot(planePos, clippingPlanes[i].normal);
          float dist0 = dot(clippingPlanes[i].normal, p0->rpos) - d;
          float dist1 = dot(clippingPlanes[i].normal, p1->rpos) - d;
          float factor = dist0 / (fabsf(dist0) + fabsf(dist1));
          vec3 intersectionPoint(p0->rpos + factor * (p1->rpos - p0->rpos));

          Vertex* intersectionVert = &clipBufferVertex[clipBufferCounter++];
          intersectionVert->rpos = intersectionPoint;
          curUVDestList[currentDestCount] = curUVSourceList[e] + (factor * (curUVSourceList[p1id] - curUVSourceList[e]));

          curDestList[currentDestCount++] = intersectionVert;
        }
        else if (!firstIn && secondIn) // Going in
        {
          float d = dot(planePos, clippingPlanes[i].normal);
          float dist0 = dot(clippingPlanes[i].normal, p0->rpos) - d;
          float dist1 = dot(clippingPlanes[i].normal, p1->rpos) - d;
          float factor = dist1 / (fabsf(dist0) + fabsf(dist1));
          vec3 intersectionPoint(p1->rpos + factor * (p0->rpos - p1->rpos));

          Vertex* intersectionVert = &clipBufferVertex[clipBufferCounter++];
          intersectionVert->rpos = intersectionPoint;
          curUVDestList[currentDestCount] = curUVSourceList[p1id] + (factor * (curUVSourceList[e] - curUVSourceList[p1id]));
          curDestList[currentDestCount++] = intersectionVert;

          curUVDestList[currentDestCount] = curUVSourceList[p1id];
          curDestList[currentDestCount++] = p1;
        }
      }
    }
    clipBufferCounter = 0;

    if (currentDestCount == 0)
      continue;

    int startY = SCRHEIGHT - 1, endY = 0;

    const static float  scrwAddition = (SCRWIDTH / 10.0f) * ASPECT_RATIO;
    const static float  scrhAddition = (SCRHEIGHT / 10.0f);
    // Setup screen coords
    for (uint i = 0; i < currentDestCount; ++i)
    {
      screenCoords[i] = vec2(curDestList[i]->rpos);

      screenCoords[i].x =
        screenCoords[i].x / (curDestList[i]->rpos.z * 0.1f)
        * scrwAddition + HALF_SCRW;

      screenCoords[i].y =
        screenCoords[i].y / (curDestList[i]->rpos.z * 0.1f)
        * scrhAddition + HALF_SCRH;
    }

    for (uint i = 0; i < currentDestCount; ++i)
    {
      Vertex* v0 = curDestList[i];
      uint v0id = i;
      uint v1id = (i + 1) % currentDestCount;
      Vertex* v1 = curDestList[v1id];

      if (screenCoords[v1id].y < screenCoords[v0id].y)
      {
        Vertex* temp = v0;
        v0 = v1;
        v1 = temp;

        uint uiTemp = v0id;
        v0id = v1id;
        v1id = uiTemp;
      }
      // Setup pixel vars
      const float y1 = screenCoords[v0id].y, y2 = screenCoords[v1id].y;

      if (y2 < 0 || (y2 == y1) || y1 > SCRHEIGHT - 1)
        continue;

      int iy1 = (int)(y1 + 1.0f), iy2 = (int)y2;
      iy1 = iy1 > 0 ? iy1 : 0;

      // Setup additional vars
      float x1 = screenCoords[v0id].x, z1 = 1.0f / (v0->rpos.z);
      float UVu1 = curUVDestList[v0id].x * z1, UVv1 = curUVDestList[v0id].y * z1;

      const float x2 = screenCoords[v1id].x, z2 = 1.0f / (v1->rpos.z);
      const float UVu2 = curUVDestList[v1id].x * z2, UVv2 = curUVDestList[v1id].y * z2;

      iy2 = iy2 > SCRHEIGHT - 1 ? SCRHEIGHT - 1 : iy2;

      startY = iy1 < startY ? iy1 : startY, endY = iy2 > endY ? iy2 : endY;

      // Lerp values
      const float yDiff = 1.0f / (y2 - y1);
      const float dx = (x2 - x1) * yDiff; // x stepsize
      const float dz = (z2 - z1) * yDiff; // z stepsize
      const float du = (UVu2 - UVu1) * yDiff; // u stepsize
      const float dv = (UVv2 - UVv1) * yDiff; // v stepsize

      // Sub-pixel corrections
      const float sPC = ((float)iy1 - y1);
      x1 += dx * sPC, z1 += dz * sPC, UVu1 += du * sPC, UVv1 += dv * sPC;

      // Fill outline tables
      // Loop over edges
      float* pMM = &(minsmaxs[iy1 * MINMAXEL]);
      for (int y = iy1; y <= iy2; ++y)
      {
        if (x1 < pMM[XMIN_IDX])
          pMM[XMIN_IDX] = x1, pMM[ZMIN_IDX] = z1, pMM[UMIN_IDX] = UVu1, pMM[VMIN_IDX] = UVv1;

        if (x1 > pMM[XMAX_IDX])
          pMM[XMAX_IDX] = x1, pMM[ZMAX_IDX] = z1, pMM[UMAX_IDX] = UVu1, pMM[VMAX_IDX] = UVv1;
        
        pMM += MINMAXEL;
        x1 += dx, z1 += dz, UVu1 += du, UVv1 += dv;
      }
    }
    ++g_trianglesDrawn;
    const uint txwmin1 = (g_currentColorMap->width - 1);
    const uint txhmin1 = (g_currentColorMap->height - 1);
    // Draw spans
    float* pMM = &minsmaxs[startY * MINMAXEL];

    static float z1, u1, v1, z2, u2, v2, xDiff, dz, du, dv;

    for (int y = startY; y <= endY; ++y, pMM += MINMAXEL)
    {
      int ix1 = (int)(pMM[XMIN_IDX] + 1.0f), ix2 = (int)(pMM[XMAX_IDX]);

      xDiff = 1.0f / (pMM[XMAX_IDX] - pMM[XMIN_IDX]);

      z1 = pMM[ZMIN_IDX], u1 = pMM[UMIN_IDX], v1 = pMM[VMIN_IDX];
      z2 = pMM[ZMAX_IDX], u2 = pMM[UMAX_IDX], v2 = pMM[VMAX_IDX];
      dz = (z2 - z1) * xDiff, du = (u2 - u1) * xDiff, dv = (v2 - v1) * xDiff;

      // Sub-texel correction on UV values
      const float subTexCorr = ((float)ix1 - pMM[XMIN_IDX]);
      z1 += dz * subTexCorr, v1 += dv * subTexCorr, u1 += du * subTexCorr;\

      unsigned long* screenBuffer = &g_screen[y*SCRWIDTH + ix1];
      float* zBuffer = &zbuffer[y * SCRWIDTH + ix1];
      if (filtered)
      {
        for (int x = ix1; x <= ix2; ++x)
        {
          const float recipZ = 1.0f / z1;
          const uint uCoord = (uint)(u1 * recipZ * g_currentColorMap->fwidth * 16.0f);
          const uint vCoord = (uint)(v1 * recipZ * g_currentColorMap->fheight * 16.0f);
          const uint uCoordShifted = (uCoord >> 4) & txwmin1;
          const uint vCoordShifted = (vCoord >> 4) & txhmin1;

          const uint col1 = g_currentColorBuffer[Texture::mortonX[((uCoord >> 4)) & txwmin1] | Texture::mortonY[((vCoord >> 4)) & txhmin1]];
          const uint col2 = g_currentColorBuffer[Texture::mortonX[((uCoord >> 4) + 1) & txwmin1] | Texture::mortonY[((vCoord >> 4))]];
          const uint col3 = g_currentColorBuffer[Texture::mortonX[((uCoord >> 4)) & txwmin1] | Texture::mortonY[((vCoord >> 4) + 1) & txhmin1]];
          const uint col4 = g_currentColorBuffer[Texture::mortonX[((uCoord >> 4) + 1) & txwmin1] | Texture::mortonY[((vCoord >> 4) + 1) & txhmin1]];

          const uint uvLookup = (uCoord & 0xF) + ((vCoord & 0xF) << 4);

          const uint rb =
            (((col1 & REDBLUEMASK) * bilinearFilteringPrecalcs[uvLookup][0]) >> 8) +
            (((col2 & REDBLUEMASK) * bilinearFilteringPrecalcs[uvLookup][1]) >> 8) +
            (((col3 & REDBLUEMASK) * bilinearFilteringPrecalcs[uvLookup][2]) >> 8) +
            (((col4 & REDBLUEMASK) * bilinearFilteringPrecalcs[uvLookup][3]) >> 8);

          const uint g =
            (((col1 & GREENMASK) * bilinearFilteringPrecalcs[uvLookup][0]) >> 8) +
            (((col2 & GREENMASK) * bilinearFilteringPrecalcs[uvLookup][1]) >> 8) +
            (((col3 & GREENMASK) * bilinearFilteringPrecalcs[uvLookup][2]) >> 8) +
            (((col4 & GREENMASK) * bilinearFilteringPrecalcs[uvLookup][3]) >> 8);

          *screenBuffer = (rb & 0xFF00FF) | (g & 0xFF00), *zBuffer = 0;

          ++zBuffer, ++screenBuffer, z1 += dz, u1 += du, v1 += dv;
        }
      }
      else
      {
        for (int x = ix1; x <= ix2; ++x)
        {
          const float recipZ = 1.0f / z1; // is 1/1/Z
          const uint uCoord = (uint)(u1 * recipZ * g_currentColorMap->fwidth) & txwmin1;
          const uint vCoord = (uint)(v1 * recipZ * g_currentColorMap->fheight) & txhmin1;
          
          const uint col = g_currentColorBuffer[Texture::mortonX[uCoord] | Texture::mortonY[vCoord]];

          *screenBuffer = col, *zBuffer = 0;

          ++zBuffer, ++screenBuffer, z1 += dz, u1 += du, v1 += dv;
        }
      }
      pMM[XMIN_IDX] = SCRWIDTH - 1, pMM[XMAX_IDX] = 0;
    }
  }
}
