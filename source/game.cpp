#include "InputManager.h"
#include "template.h"
#include "surface.h"
#include "game.h"

#include "TextureManager.h"
#include "MeshManager.h"
#include "MaterialManager.h"
#include "LightManager.h"
#include "SoundManager.h"

#include "Renderer.h"
#include "Triangle.h"
#include "GameObject.h"

#include "Skybox.h"
#include "Spaceship.h"
#include "Camera.h"
#include "Asteroid.h"
#include <time.h>

using namespace Tmpl8;

vec3 mouseMovement, prevMouseMovement;
vec3 objPos;
float r = 0;
int Game::score = 0;
int Game::asteroids = 0;
int Game::livesLeft = 0;

static unsigned int dragonMeshId = 0;
static bool gameMode = true;

static bool bilinFiltering = true;

void Game::Init()
{
  Texture::InitMortonCurveLookupTable();

  Lights->lightColor = 0xFFFFFF;
  Lights->ambientTerm = 25;
  Lights->diffuseTerm = 1.0f;
  Lights->lightDir = glm::normalize(glm::vec3(-1, 2, 1));

  Renderer::g_screen = m_Surface->GetBuffer();
  Renderer::Init();

  Sounds->Add("shoot");
  Sounds->Add("explosion");

  Textures->Add("notex", "resources/notex.png");
  Textures->Add("uv_map_reference", "resources/uv_map_reference.jpg");

  Meshes->Add("nomesh", "resources/nomesh");
  uint spaceShipId = Meshes->Add("spaceship", "resources/spaceship");
  Textures->Get(Materials->Get(Meshes->Get(spaceShipId)->m_material)->texture)->AddTint(0xFF4444);

  Meshes->Add("asteroid_big", "resources/asteroid_big");
  Meshes->Add("asteroid_medium", "resources/asteroid_medium");
  Meshes->Add("asteroid_small", "resources/asteroid_small");
  Meshes->Add("bullet", "resources/bullet");
  dragonMeshId = Meshes->Add("dragon", "resources/dragon");
  
  auto skyboxId = Meshes->Add("skybox", "resources/skybox");

  skybox = new Skybox();
  skybox->mesh = Meshes->Get(skyboxId);

  SetupGame();
}

void Game::SetupGame()
{
  state = GAME_UNSTARTED;
  m_gameStartTimer = 0;

  srand(time(0));

  score = 0;
  asteroids = 0;
  livesLeft = 3;

  camera = new Camera();

  sceneRoot = new GameObject();
  sceneRoot->AddChild(std::shared_ptr<GameObject>(new Spaceship()));
  sceneRoot->AddChild(std::shared_ptr<GameObject>(new GameObject()));
  sceneRoot->pendingChildren[1]->transform.Translate(vec3(0, 5, 20));
  //sceneRoot->AddChild(std::shared_ptr<GameObject>(new Asteroid(0, vec3(0,0,5))));
  for (uint i = 0; i < 100; ++i)
    sceneRoot->AddChild(std::shared_ptr<GameObject>(new Asteroid(0, vec3(rand() % 256 - 128, rand() % 32 - 16, rand() % 256 - 128))));

  sceneRoot->pendingChildren[0]->mesh = Meshes->GetID("spaceship");

  sceneRoot->pendingChildren[0]->AddChild(std::shared_ptr<GameObject>(camera));
  camera->player = (Spaceship*)sceneRoot->pendingChildren[0].get();

  Materials->Get(Meshes->Get(sceneRoot->pendingChildren[0]->mesh)->m_material)->color = 0xFFFFFF;
}

void Game::Shutdown()
{
  delete skybox;
  delete sceneRoot;
  //delete camera;
  CollisionManager::Reset();
  SoundManager::Reset();
  TextureManager::Reset();
  MaterialManager::Reset();
  MeshManager::Reset();
  InputManager::Reset();
}


void Game::Tick(float a_fDeltaTime)
{
  static char textBuffer[128];
  //memset(m_Surface->GetBuffer(), 0, SCRWIDTH * SCRHEIGHT * 4);

  if (Input->IsKeyPressed(SDLK_p) && state == GAME_PLAYING)
  {
    sceneRoot->children[1]->mesh = sceneRoot->children[1]->mesh == 0 ? dragonMeshId : 0;
    gameMode = !gameMode;
  }
  if (gameMode)
  {
    switch (state)
    {
    case GAME_UNSTARTED:{
      camera->CalculateMatrix();

      auto invCam = inverse(Camera::g_transform);
      Lights->lightDirTransformed = Lights->lightDir * mat3(invCam);

      skybox->Render(invCam);

      sprintf_s(textBuffer, "\'wasd\' to control the ship");
      m_Surface->Print(textBuffer, SCRWIDTH / 2 - strlen(textBuffer) * 3, SCRHEIGHT / 2, 0xFF0000);
      sprintf_s(textBuffer, "\'space\' to accelerate");
      m_Surface->Print(textBuffer, SCRWIDTH / 2 - strlen(textBuffer) * 3, SCRHEIGHT / 2 + 12, 0xFF0000);
      sprintf_s(textBuffer, "\'left ctrl\' to shoot");
      m_Surface->Print(textBuffer, SCRWIDTH / 2 - strlen(textBuffer) * 3, SCRHEIGHT / 2 + 24, 0xFF0000);
      sprintf_s(textBuffer, "press \'space\' to start the game");
      m_Surface->Print(textBuffer, SCRWIDTH / 2 - strlen(textBuffer) * 3, SCRHEIGHT / 2 + 96, 0xFF0000);

      if (Input->IsKeyPressed(SDLK_SPACE))
      {
        state = GAME_STARTING;
      }
      break;
    }
    case GAME_STARTING:{
      camera->CalculateMatrix();

      auto invCam = inverse(Camera::g_transform);
      Lights->lightDirTransformed = Lights->lightDir * mat3(invCam);

      skybox->Render(invCam);

      sprintf_s(textBuffer, "%i", 3 - (int)m_gameStartTimer);
      m_Surface->Print(textBuffer, SCRWIDTH / 2 - 3, SCRHEIGHT / 2 - 3, 0xFF0000);

      m_gameStartTimer += a_fDeltaTime;
      if (m_gameStartTimer > 4)
      {
        m_gameStartTimer = 0;
        state = GAME_PLAYING;
      }
      break;
    }
    case GAME_OVER:
      sprintf_s(textBuffer, "your score is %i", score);
      m_Surface->Print(textBuffer, SCRWIDTH / 2 - strlen(textBuffer) * 3, SCRHEIGHT / 2 - 3, 0xFF0000);
      sprintf_s(textBuffer, "press \'r\' to restart");
      m_Surface->Print(textBuffer, SCRWIDTH / 2 - strlen(textBuffer) * 3, SCRHEIGHT / 2 + 9, 0xFF0000);
      if (Input->IsKeyPressed(SDLK_r))
      {
        delete sceneRoot;
        SetupGame();
      }
      break;
    case GAME_PLAYING:{
      sceneRoot->Update(a_fDeltaTime);
      camera->CalculateMatrix();

      auto invCam = inverse(Camera::g_transform);
      Lights->lightDirTransformed = Lights->lightDir * mat3(invCam);

      skybox->Render(invCam, bilinFiltering);
      sceneRoot->Render(invCam);

      CollisionManager::GetInstance()->Update(a_fDeltaTime);

      // Crosshair
      m_Surface->Line(SCRWIDTH / 2 - 10, SCRHEIGHT / 2, SCRWIDTH / 2 + 10, SCRHEIGHT / 2, 0x3333DD);
      m_Surface->Line(SCRWIDTH / 2, SCRHEIGHT / 2 - 10, SCRWIDTH / 2, SCRHEIGHT / 2 + 10, 0x3333DD);

      sprintf_s(textBuffer, "score: %i", score);
      m_Surface->Print(textBuffer, 10, 10, 0xFFFFFF);

      sprintf_s(textBuffer, "asteroids left: %i", asteroids);
      m_Surface->Print(textBuffer, 10, 22, 0xFFFFFF);

      sprintf_s(textBuffer, "lives left: %i", livesLeft);
      m_Surface->Print(textBuffer, 10, 34, 0xFFFFFF);

      sprintf_s(textBuffer, "press \'p'\ for pause (and dragon)");
      m_Surface->Print(textBuffer, SCRWIDTH - strlen(textBuffer) * 6 - 10, 10, 0xFFFF00);

      if (livesLeft <= 0)
      {
        livesLeft = 0;
        state = GAME_OVER;
      }

      if (Input->IsKeyPressed(SDLK_KP_MINUS))
      {
        --livesLeft;
      }

      break;
    }
    }
  }
  else
  {
    sceneRoot->children[0]->Update(a_fDeltaTime);
    sceneRoot->children[1]->Update(a_fDeltaTime);
    camera->CalculateMatrix();

    auto invCam = inverse(Camera::g_transform);
    Lights->lightDirTransformed = Lights->lightDir * mat3(invCam);

    skybox->Render(invCam, bilinFiltering);
    sceneRoot->children[1]->Render(invCam);

    sprintf_s(textBuffer, "press \'p'\ to continue playing");
    m_Surface->Print(textBuffer, SCRWIDTH - strlen(textBuffer) * 6 - 10, 10, 0xFFFF00);
    sprintf_s(textBuffer, "press \'b'\ to toggle bilinear filtering on skybox");
    m_Surface->Print(textBuffer, SCRWIDTH - strlen(textBuffer) * 6 - 10, 22, 0xFFFF00);
  }

  if (Input->IsKeyDown(SDLK_k))
  {
    Lights->diffuseTerm -= a_fDeltaTime;
  }
  if (Input->IsKeyDown(SDLK_l))
  {
    Lights->diffuseTerm += a_fDeltaTime;
  }
  if (Input->IsKeyDown(SDLK_LEFT))
  {
    Lights->lightDir.x -= a_fDeltaTime;
    Lights->lightDir = normalize(Lights->lightDir);
  }
  if (Input->IsKeyDown(SDLK_RIGHT))
  {
    Lights->lightDir.x += a_fDeltaTime;
    Lights->lightDir = normalize(Lights->lightDir);
  }
  if (Input->IsKeyDown(SDLK_UP))
  {
    Lights->lightDir.z += a_fDeltaTime;
    Lights->lightDir = normalize(Lights->lightDir);
  }
  if (Input->IsKeyDown(SDLK_DOWN))
  {
    Lights->lightDir.z -= a_fDeltaTime;
    Lights->lightDir = normalize(Lights->lightDir);
  }
  if (Input->IsKeyPressed(SDLK_b))
  {
    bilinFiltering = !bilinFiltering;
  }


  sprintf_s(textBuffer, "tri count: %u", Renderer::g_totalTriangles);
  m_Surface->Print(textBuffer, 10, SCRHEIGHT - 36, 0xFFFF00);

  sprintf_s(textBuffer, "tris drawn: %u", Renderer::g_trianglesDrawn);
  m_Surface->Print(textBuffer, 10, SCRHEIGHT - 24, 0xFFFF00);

  sprintf_s(textBuffer, "tris culled: %u", Renderer::g_trianglesCulled);
  m_Surface->Print(textBuffer, 10, SCRHEIGHT - 12, 0xFFFF00);

  Renderer::g_totalTriangles = Renderer::g_trianglesDrawn = Renderer::g_trianglesCulled = 0;
  Renderer::ClearZBuffer();

  sprintf_s(textBuffer, "%f %f %f", camera->player->transform.GetPosition().x, camera->player->transform.GetPosition().y, camera->player->transform.GetPosition().z);
  m_Surface->Print(textBuffer, 10, SCRHEIGHT - 48, 0xFFFF00);
}