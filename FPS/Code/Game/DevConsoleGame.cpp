#include "Game/GameCommon.hpp"
#include "Game/DevConsoleGame.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include <string>


BitmapFont*				g_devConsoleFont = nullptr;


//---------------------------------------------------------------------------------------------------------
DevConsoleGame::~DevConsoleGame()
{
}

//---------------------------------------------------------------------------------------------------------
DevConsoleGame::DevConsoleGame()
{
}


//---------------------------------------------------------------------------------------------------------
void DevConsoleGame::StartUp()
{
	g_theConsole->PrintString( Rgba8::BLUE, "Dev Console Start Up" );
	m_devConsoleCamera.SetOrthoView( Vec2( -HALF_SCREEN_X, -HALF_SCREEN_Y ), Vec2( HALF_SCREEN_X, HALF_SCREEN_Y ) );

	g_devConsoleFont = g_theRenderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" );
}


//---------------------------------------------------------------------------------------------------------
void DevConsoleGame::ShutDown()
{
}


//---------------------------------------------------------------------------------------------------------
void DevConsoleGame::Render() const
{
	//Render Dev Console
	if( !g_theConsole->IsOpen() )
	{
		return;
	}

	g_theRenderer->BeginCamera( m_devConsoleCamera );
	g_theConsole->Render( *g_theRenderer, m_devConsoleCamera, DEV_CONSOLE_LINE_HEIGHT, g_devConsoleFont );
	RenderCurrentInput();
	g_theRenderer->EndCamera( m_devConsoleCamera );
}


//---------------------------------------------------------------------------------------------------------
void DevConsoleGame::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if( m_isDevConsoleOpen )
	{
		ProcessInput();

		if( !g_theConsole->IsOpen() )
		{
			m_currentInput = "";
		}
	}

	m_isDevConsoleOpen = g_theConsole->IsOpen();
}


//---------------------------------------------------------------------------------------------------------
void DevConsoleGame::RenderCurrentInput() const
{
	if( m_currentInput == "" ) return;

	std::vector<Vertex_PCU> inputVerts;
	g_devConsoleFont->AddVertsForText2D( inputVerts, m_devConsoleCamera.GetOrthoBottomLeft(), DEV_CONSOLE_LINE_HEIGHT, m_currentInput );

	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( inputVerts );
}


//---------------------------------------------------------------------------------------------------------
void DevConsoleGame::ProcessInput()
{
	if( HandleKeyPresses() ) return;

	char inputCharacter;
	while( g_theInput->PopFromCharacterQueue( &inputCharacter ) )
	{
		AddCharacterToInput( inputCharacter );
	}
}


//---------------------------------------------------------------------------------------------------------
void DevConsoleGame::AddCharacterToInput( char c )
{
	m_currentInput += Stringf( "%c", c );
}


//---------------------------------------------------------------------------------------------------------
void DevConsoleGame::HandleEscapeKey()
{
	if( m_currentInput != "" )
	{
		m_currentInput = "";
	}
	else
	{
		g_theConsole->ToggleIsOpen();
	}
}


//---------------------------------------------------------------------------------------------------------
bool DevConsoleGame::HandleKeyPresses()
{
	if( g_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		HandleEscapeKey();
		return true;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_ENTER ) )
	{
		SubmitCommand();
		return true;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_BACKSPACE ) )
	{
		size_t newStringSize = m_currentInput.length() - 1;
		m_currentInput = m_currentInput.substr( 0, newStringSize );
		return true;
	}

	if( g_theInput->WasKeyJustPressed( KEY_CODE_DELETE ) )
	{
		m_currentInput = m_currentInput.substr( 1, m_currentInput.length() );
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
void DevConsoleGame::SubmitCommand()
{
	std::string commandString = m_currentInput + " is not a supported command";
	g_theConsole->PrintString( Rgba8::RED, commandString );
	m_currentInput = "";
}
