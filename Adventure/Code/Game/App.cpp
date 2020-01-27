#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
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


EventSystem*	g_theEventSystem = nullptr;
RenderContext* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
DevConsole* g_theConsole = nullptr;
Game* g_theGame = nullptr;


//---------------------------------------------------------------------------------------------------------
void App::StartUp()
{
	g_theEventSystem = new EventSystem();
	g_theRenderer = new RenderContext();
	g_theInput = new InputSystem();
	g_theAudio = new AudioSystem();
	g_theConsole = new DevConsole();
	g_theGame = new Game();

	g_theEventSystem->StartUp();
	g_theRenderer->StartUp( g_theWindow );
	
	g_theInput->StartUp();
	g_theWindow->SetInputSystem( g_theInput );

	g_theConsole->StartUp();
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

	delete g_theConsole;
	g_theConsole = nullptr;

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
	if( g_theGame->IsQuitting() )
	{
		HandleQuitRequested();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		HandleQuitRequested();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F8 ) )
	{
		RestartGame();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F4 ) )
	{
		g_isDebugCamera = !g_isDebugCamera;
	}

	g_theGame->Update( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void App::Render() const
{
	//g_theRenderer->ClearScreen( Rgba8::RED );

	g_theGame->Render();
}


//---------------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();
}
