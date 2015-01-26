#pragma once
#include <map>
#include "Singleton.h"
#include "SDL_keyboard.h"
#include "SDL_mouse.h"

#include "glm/glm.hpp"
//using namespace glm;

struct vector2
{
  float x, y;
};

class InputManager : public Singleton<InputManager>
{
public:
	~InputManager(void);
	InputManager(void);
	
  bool IsKeyPressed(SDL_Keycode a_key);
  bool IsKeyDown(SDL_Keycode a_key);
  bool IsKeyUp(SDL_Keycode a_key);
	
  void SetKeyPressed(SDL_Keycode a_key);
  void SetKeyUp(SDL_Keycode a_key);

  bool IsMouseButtonPressed(Uint8 a_mouseButton){ return m_mouseButtonPressed[a_mouseButton]; }
  bool IsMouseButtonDown(Uint8 a_mouseButton){ return m_mouseButtonDown[a_mouseButton]; }
  bool IsMouseButtonUp(Uint8 a_mouseButton){ return m_mouseButtonUp[a_mouseButton]; }
	
	void SetMouseButtonPressed(Uint8 a_mouseButton);
	void SetMouseButtonUp(Uint8 a_mouseButton);
	
  void SetMousePosition(float a_X, float a_Y)
  {
    m_mousePosition.x = a_X;
    m_mousePosition.y = a_Y;
  }


  void SetMouseMovement(float a_X, float a_Y) 
  { 
    m_mouseMovement.x = a_X;
    m_mouseMovement.y = a_Y;
  }

  glm::vec2 GetMousePosition() { return m_mousePosition; }
  
  void Update(float a_fDeltaTime);
private:
	
	glm::vec2 m_mousePosition;
  glm::vec2 m_mouseMovement;
	
  std::map<SDL_Keycode, bool> m_keysDown;
  std::map<SDL_Keycode, bool> m_keysPressed;
  std::map<SDL_Keycode, bool> m_keysUp;
	
	std::map<Uint8, bool> m_mouseButtonDown;
	std::map<Uint8, bool> m_mouseButtonPressed;
	std::map<Uint8, bool> m_mouseButtonUp;
};

#define Input InputManager::GetInstance()