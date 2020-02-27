#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include "Game/App.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Platform/Window.hpp"


App* g_theApp = nullptr;
Window* g_theWindow = nullptr;

//-----------------------------------------------------------------------------------------------
// #SD1ToDo: Move each of these items to its proper place, once that place is established
//
const char* APP_NAME = "Physics System Test";					// ...becomes ??? (Change this per project!)


												//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( _In_ HINSTANCE applicationInstanceHandle, _In_opt_ HINSTANCE, _In_ LPSTR commandLineString, _In_ int )
{
	UNUSED( commandLineString );
	UNUSED( applicationInstanceHandle );

	g_theWindow = new Window();
	g_theWindow->Open( APP_NAME, CLIENT_ASPECT );

	g_theApp = new App();
	g_theApp->StartUp();


	XmlDocument gameConfigFile = new XmlDocument();
	gameConfigFile.LoadFile( "Data/GameConfig.xml" );
	GUARANTEE_OR_DIE( gameConfigFile.ErrorID() == 0, "GameConfig.xml does not exist in Run/Data" );
	g_gameConfigBlackboard.PopulateFromXmlElementAttribute( *gameConfigFile.RootElement() );


	// Program main loop; keep running frames until it's time to quit
	while( !g_theApp->IsQuitting() )
	{
		g_theWindow->BeginFrame();
		g_theApp->RunFrame();
	}

	g_theApp->ShutDown();
	delete g_theApp;
	g_theApp = nullptr;

	g_theWindow->Close();
	delete g_theWindow;
	g_theWindow = nullptr;

	return 0;
}