#include "InputManager.h"


InputManager::InputManager(void)
{
}


InputManager::~InputManager(void)
{
}


bool InputManager::IsKeyPressed(SDL_Keycode a_key){return m_keysPressed[a_key];}
bool InputManager::IsKeyDown(SDL_Keycode a_key){return m_keysDown[a_key];}
bool InputManager::IsKeyUp(SDL_Keycode a_key){return m_keysUp[a_key];}

void InputManager::SetKeyPressed(SDL_Keycode a_key){ m_keysUp[a_key] = false; if (!m_keysDown[a_key]){ m_keysDown[a_key] = true; m_keysPressed[a_key] = true; } }
void InputManager::SetKeyUp(SDL_Keycode a_key){m_keysUp[a_key] = true; m_keysDown[a_key] = false;}

void InputManager::Update(float a_fDeltaTime)
{
	for(auto iter = m_keysPressed.begin(); iter != m_keysPressed.end(); iter++)
	{
		iter->second = false;
	}	
  for (auto iter = m_keysUp.begin(); iter != m_keysUp.end(); iter++)
	{
		iter->second = false;
	}

  for (auto iter = m_mouseButtonPressed.begin(); iter != m_mouseButtonPressed.end(); iter++)
	{
		iter->second = false;
	}	
  for (auto iter = m_mouseButtonUp.begin(); iter != m_mouseButtonUp.end(); iter++)
	{
		iter->second = false;
	}
}

void InputManager::SetMouseButtonPressed(Uint8 a_mouseButton)
{
	m_mouseButtonUp[a_mouseButton] = false; 
  if (!m_mouseButtonDown[a_mouseButton])
  {
    m_mouseButtonDown[a_mouseButton] = true;
    m_mouseButtonPressed[a_mouseButton] = true;
  }
}

void InputManager::SetMouseButtonUp(Uint8 a_mouseButton)
{
	m_mouseButtonUp[a_mouseButton] = true; 
	m_mouseButtonDown[a_mouseButton] = false;
}