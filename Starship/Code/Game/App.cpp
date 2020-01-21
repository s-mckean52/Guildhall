#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Game/PlayerShip.hpp"


RenderContext* g_theRenderer = nullptr;
Game* g_theGame = nullptr;
InputSystem* g_theInput = nullptr;

//---------------------------------------------------------------------------------------------------------
void App::StartUp()
{
	g_theRenderer = new RenderContext();
	g_theGame = new Game();
	g_theInput = new InputSystem();

	g_theRenderer->StartUp();
	g_theGame->StartUp();
	g_theInput->StartUp();
}


//---------------------------------------------------------------------------------------------------------
void App::ShutDown()
{
	g_theRenderer->ShutDown();
	delete g_theRenderer;
	g_theRenderer = nullptr;

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
	g_theGame->SetGameState( QUITTING_STATE );
	m_isQuitting = true;

	return 0;
}


//---------------------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	g_theInput->BeginFrame();

	if( g_theGame->GetGameState() == RESTARTING_STATE )
	{
		RestartGame();
	}

	for( int controllerIndex = 0; controllerIndex < MAX_NUM_PLAYERS; ++controllerIndex )
	{
		const KeyButtonState& startButton = g_theInput->GetXboxController( controllerIndex ).GetButtonState( XBOX_BUTTON_ID_START );
		if( startButton.WasJustPressed() )
		{
			g_theGame->SetGameState( PLAY_STATE );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void App::Update( float deltaSeconds )
{
	if( m_isPaused )
	{
		deltaSeconds *= 0.f;
	}

	if( m_isSlowMo )
	{
		deltaSeconds *= 0.1f;
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
