#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
//#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Platform/Window.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"


JobSystem*		g_theJobSystem		= nullptr;
EventSystem*	g_theEventSystem	= nullptr;
RenderContext*	g_theRenderer		= nullptr;
InputSystem*	g_theInput			= nullptr;
AudioSystem*	g_theAudio			= nullptr;
DevConsole*		g_theConsole		= nullptr;
Game*			g_theGame			= nullptr;


//---------------------------------------------------------------------------------------------------------
void App::StartUp()
{
	Clock::SystemStartUp();

	g_theJobSystem		= new JobSystem();
	g_theEventSystem	= new EventSystem();
	g_theRenderer		= new RenderContext();
	g_theInput			= new InputSystem();
	g_theAudio			= new AudioSystem();
	g_theConsole		= new DevConsole();
	g_theGame			= new Game();

	g_theEventSystem->StartUp();
	g_theRenderer->StartUp( g_theWindow );
	g_theInput->StartUp( g_theWindow );
	g_theConsole->StartUp( g_theInput, g_theEventSystem );

	DebugRenderSystemStartup( g_theRenderer );

	g_theGame->StartUp();
	
	g_theWindow->SetInputSystem( g_theInput );
	g_theWindow->SetEventSystem( g_theEventSystem );

	g_theEventSystem->SubscribeEventCallbackFunction( "quit", QuitRequested );
	g_theEventSystem->SubscribeEventCallbackFunction( "help", HelpCommand );

	m_devConsoleCamera = new Camera( g_theRenderer );
	m_devConsoleCamera->SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );
}


//---------------------------------------------------------------------------------------------------------
void App::ShutDown()
{
 	delete m_devConsoleCamera;
 	m_devConsoleCamera = nullptr;

	DebugRenderSystemShutdown();

	g_theGame->ShutDown();
	delete g_theGame;
	g_theGame = nullptr;

	g_theEventSystem->ShutDown();
	delete g_theEventSystem;
	g_theEventSystem = nullptr;

	g_theInput->ShutDown();
	delete g_theInput;
	g_theInput = nullptr;

	delete g_theAudio;
	g_theAudio = nullptr;

	g_theConsole->ShutDown();
	delete g_theConsole;
	g_theConsole = nullptr;

	g_theJobSystem->ShutDown();
	delete g_theJobSystem;
	g_theJobSystem = nullptr;

	Clock::SystemShutdown();

	g_theRenderer->ShutDown();
	delete g_theRenderer;
	g_theRenderer = nullptr;

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
STATIC void App::QuitRequested( EventArgs* args )
{
	UNUSED( args );
	g_theApp->HandleQuitRequested();
}


//---------------------------------------------------------------------------------------------------------
STATIC void App::HelpCommand( EventArgs* args )
{
	UNUSED( args );
	Strings registeredCommands = g_theEventSystem->GetEventNames();
	for( int commandIndex = 0; commandIndex < registeredCommands.size(); ++commandIndex )
	{
		g_theConsole->PrintString( Rgba8::YELLOW, registeredCommands[ commandIndex ] );
	}
}

//---------------------------------------------------------------------------------------------------------
void App::RunFrame()
{
	BeginFrame(); //All engine systems 
	Update(); //only for the game
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
	Clock::BeginFrame();

	g_theRenderer->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();

	DebugRenderBeginFrame();
}


//---------------------------------------------------------------------------------------------------------
void App::Update()
{
	g_theGame->Update();
	g_theConsole->Update();

	if( g_theGame->IsQuitting() || g_theWindow->IsQuitting() )
	{
		HandleQuitRequested();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F8 ) )
	{
		RestartGame();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_F9 ) )
	{
		g_theRenderer->ReloadShaders();
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_TILDE ) )
	{
		g_theConsole->ToggleIsOpen();
	}
}


//---------------------------------------------------------------------------------------------------------
void App::Render() const
{
	g_theGame->Render();
	DebugRenderScreenTo( g_theRenderer->GetBackBuffer() );
	g_theConsole->Render( *g_theRenderer, *m_devConsoleCamera, DEV_CONSOLE_LINE_HEIGHT, g_devConsoleFont );
}


//---------------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_theRenderer->EndFrame();
	g_theInput->EndFrame();
	g_theAudio->EndFrame();

	DebugRenderEndFrame();
}
