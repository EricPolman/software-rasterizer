#pragma once
#include "definitions.h"
#include "Camera.h"

class GameObject;
class Skybox;

enum GameState
{
  GAME_UNSTARTED,
  GAME_STARTING,
  GAME_PLAYING,
  GAME_OVER
};

namespace Tmpl8 {

class Surface;
class Game
{
public:
	void SetTarget( Surface* a_Surface ) { m_Surface = a_Surface; }
	void Init();
	void DrawTri( float x1, float y1, float x2, float y2, float x3, float y3 );
	void Tick( float a_DT );
	void Shutdown();
	void MouseButton( int a_Nr, bool a_Down ) {}
	void MouseMove( int a_X, int a_Y ) {}
	void KeyUp( int a_Key ) {}
	void KeyDown( int a_Key ) {}

  void SetupGame();

  static int score;
  static int asteroids;
  static int livesLeft;
private:
	Surface* m_Surface, *m_Image;

  GameObject* sceneRoot;
  Skybox* skybox;
  Camera* camera;

  GameState state;
  float m_gameStartTimer;
};

}; // namespace Tmpl8