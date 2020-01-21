#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"


RenderContext* g_theRenderer = nullptr;
InputSystem* g_theInput = nullptr;
Game* g_theGame = nullptr;

bool g_isDebugCamera = false;
bool g_isDebugDraw = false;
bool g_isNoClip = false;


//---------------------------------------------------------------------------------------------------------
void App::StartUp()
{
	g_theRenderer = new RenderContext();
	g_theInput = new InputSystem();
	g_theGame = new Game();

	g_theRenderer->StartUp();
	g_theInput->StartUp();
	g_theGame->StartUp();

}


//---------------------------------------------------------------------------------------------------------
void App::ShutDown()
{
	g_theRenderer->ShutDown();
	delete g_theRenderer;
	g_theRenderer = nullptr;

	g_theInput->ShutDown();
	delete g_theInput;
	g_theInput = nullptr;

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
	g_theInput->BeginFrame();
}


//---------------------------------------------------------------------------------------------------------
void App::Update( float deltaSeconds )
{
	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		HandleQuitRequested();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F1 ) )
	{
		g_isDebugDraw = !g_isDebugDraw;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F3 ) )
	{
		g_isNoClip = !g_isNoClip;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F4 ) )
	{
		g_isDebugCamera = !g_isDebugCamera;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F8 ) )
	{
		RestartGame();
	}

	g_theGame->Update( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void App::Render() const
{
	g_theRenderer->ClearScreen( RGBA8_BLACK );

	g_theGame->Render();
}


//---------------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_theInput->EndFrame();
}
