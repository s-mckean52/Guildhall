#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/DevConsoleGame.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Platform/Window.hpp"

EventSystem*	g_theEventSystem	= nullptr;
RenderContext*	g_theRenderer		= nullptr;
InputSystem*	g_theInput			= nullptr;
AudioSystem*	g_theAudio			= nullptr;
DevConsole*		g_theConsole		= nullptr;
DevConsoleGame*	g_theDevConsole		= nullptr;
Game*			g_theGame			= nullptr;


//---------------------------------------------------------------------------------------------------------
void App::StartUp()
{
	g_theEventSystem = new EventSystem();
	g_theRenderer = new RenderContext();
	g_theInput = new InputSystem();
	g_theAudio = new AudioSystem();
	g_theConsole = new DevConsole();
	g_theDevConsole = new DevConsoleGame();
	g_theGame = new Game();

	g_theEventSystem->StartUp();
	g_theRenderer->StartUp( g_theWindow );
	
	g_theInput->StartUp();
	g_theWindow->SetInputSystem( g_theInput );

	g_theConsole->StartUp();
	g_theDevConsole->StartUp();
	g_theGame->StartUp();

}


//---------------------------------------------------------------------------------------------------------
void App::ShutDown()
{
	g_theEventSystem->ShutDown();
	delete g_theEventSystem;
	g_theEventSystem = nullptr;

	g_theRenderer->ShutDown();
	delete g_theRenderer;
	g_theRenderer = nullptr;

	g_theInput->ShutDown();
	delete g_theInput;
	g_theInput = nullptr;

	delete g_theAudio;
	g_theAudio = nullptr;

	g_theConsole->ShutDown();
	delete g_theConsole;
	g_theConsole = nullptr;

	g_theDevConsole->ShutDown();
	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	g_theGame->ShutDown();
	delete g_theGame;
	g_theGame = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void App::RestartGame()
{
	g_theGame->ShutDown();
	delete g_theGame;
	g_theGame = nullptr;

	g_theGame = new Game();
	g_theGame->StartUp();
}


//---------------------------------------------------------------------------------------------------------
void App::RunFrame()
{
	static double timeLastFrameStarted = GetCurrentTimeSeconds();
	double timeThisFrameStarted = GetCurrentTimeSeconds();
	double deltaSeconds = timeThisFrameStarted - timeLastFrameStarted;
	timeLastFrameStarted = timeThisFrameStarted;

	if( deltaSeconds > MAX_FRAME_TIME )
	{
		deltaSeconds = MAX_FRAME_TIME;
	}

	BeginFrame(); //All engine systems 
	Update( static_cast< float >(deltaSeconds) ); //only for the game
	Render();	//only game
	EndFrame();	//all engine system
}


//---------------------------------------------------------------------------------------------------------
bool App::HandleQuitRequested()
{
	m_isQuitting = true;

	return 0;
}


//---------------------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();
}


//---------------------------------------------------------------------------------------------------------
void App::Update( float deltaSeconds )
{
	g_theRenderer->UpdateFrameTime( deltaSeconds );

	if( g_theGame->IsQuitting() || g_theWindow->IsQuitting() )
	{
		HandleQuitRequested();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F8 ) )
	{
		RestartGame();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_TILDE ) )
	{
		g_theConsole->ToggleIsOpen();
	}

	g_theGame->Update( deltaSeconds );
	g_theDevConsole->Update( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void App::Render() const
{
	g_theGame->Render();
	g_theDevConsole->Render();
}


//---------------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
}
