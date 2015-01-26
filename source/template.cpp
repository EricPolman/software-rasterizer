// Template, major revision 6
// IGAD/NHTV - Jacco Bikker - 2006-2014

// Note:
// this version of the template uses SDL2 for all frame buffer interaction
// see: https://www.libsdl.org
// for glm (OpenGL mathematics) see http://glm.g-truc.net

#pragma warning (disable : 4530) // complaint about exception handler
#pragma warning (disable : 4273)
#pragma warning (disable : 4311) // pointer truncation from HANDLE to long

extern "C" 
{ 
#include "glew.h" 
}
#include "gl.h"
#include "io.h"
#include <ios>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "SDL.h"
#include "SDL_syswm.h"
#include "wglext.h"
#include "game.h"
#include "surface.h"
#include "template.h"
#include "fcntl.h"
#include <time.h>
#include "InputManager.h"

namespace Tmpl8 { 
void NotifyUser( char* s )
{
	HWND hApp = FindWindow( NULL, "Template" );
	MessageBox( hApp, s, "ERROR", MB_OK );
	exit( 0 );
}
}

using namespace Tmpl8;
using namespace std;

static int SCRPITCH = 0;
int ACTWIDTH, ACTHEIGHT;
static bool FULLSCREEN = false, firstframe = true;

Surface* surface = 0;
Game* game = 0;
double lastftime = 0;
LARGE_INTEGER lasttime, ticksPS;

void redirectIO()
{
	static const WORD MAX_CONSOLE_LINES = 500;
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;
	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
	&coninfo);
	coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
	coninfo.dwSize);
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );
	ios::sync_with_stdio();
}

int main( int argc, char **argv ) 
{ 
  //srand(time(0));
  srand(3742);
  redirectIO();
	printf( "application started.\n" );
	SDL_Init( SDL_INIT_VIDEO );
	surface = new Surface( SCRWIDTH, SCRHEIGHT );
	surface->Clear( 0 );
	surface->InitCharset();
	SDL_Window* window = SDL_CreateWindow( "Template", 100, 100, SCRWIDTH, SCRHEIGHT, SDL_WINDOW_SHOWN );
	SDL_Renderer* renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/ );
	SDL_Texture* frameBuffer = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCRWIDTH, SCRHEIGHT );
	int exitapp = 0;
	
  clock_t startTime, endTime;
  clock_t clockTime;
  startTime = endTime = clockTime = 0;
  float fFrameTimer = 0;

  game = new Game();
  game->SetTarget(surface);

	while (!exitapp) 
	{
    endTime = clockTime;
    clockTime = clock();
    float fDeltaTime = (float)(clockTime - endTime) / CLOCKS_PER_SEC;

		void* target = 0;
		int pitch;
		SDL_LockTexture( frameBuffer, NULL, &target, &pitch );
		if (pitch == (surface->GetWidth() * 4))
		{
			memcpy( target, surface->GetBuffer(), SCRWIDTH * SCRHEIGHT * 4 );
		}
		else
		{
			unsigned char* t = (unsigned char*)target;
			for( int i = 0; i < SCRHEIGHT; i++ )
			{
				memcpy( t, surface->GetBuffer() + i * SCRWIDTH, SCRWIDTH * 4 );
				t += pitch;
			}
		}
		SDL_UnlockTexture( frameBuffer );
		SDL_RenderCopy( renderer, frameBuffer, NULL, NULL );
		SDL_RenderPresent( renderer );
		if (firstframe)
		{
			game->Init();
			firstframe = false;
		}

    game->Tick(fDeltaTime);
    Input->Update(fDeltaTime);

		// event loop
		SDL_Event event;
		while (SDL_PollEvent( &event )) 
		{
			switch (event.type)
			{
			case SDL_QUIT:
				exitapp = 1;
        break;
      case SDL_MOUSEBUTTONDOWN:
        Input->SetMouseButtonPressed(event.key.keysym.sym);
        break;
      case SDL_MOUSEBUTTONUP:
        Input->SetMouseButtonUp(event.key.keysym.sym);
        break;
      case SDL_MOUSEMOTION:
        Input->SetMousePosition((float)event.motion.x, (float)event.motion.y);
        Input->SetMouseMovement((float)event.motion.xrel, (float)event.motion.yrel);
        break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) 
				{
					exitapp = 1;
					// find other keys here: http://sdl.beuc.net/sdl.wiki/SDLKey
        }
        Input->SetKeyPressed(event.key.keysym.sym);
				game->KeyDown( event.key.keysym.scancode );
				break;
      case SDL_KEYUP:
        Input->SetKeyUp(event.key.keysym.sym);
				game->KeyUp( event.key.keysym.scancode );
				break;
			default:
				break;
			}
    }
	}
	game->Shutdown();
  delete game;
  delete surface;

  SDL_Quit();
	return 1;
}