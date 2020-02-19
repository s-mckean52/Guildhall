#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Vertex_PCU.hpp"


//---------------------------------------------------------------------------------------------------------
DevConsole::DevConsole()
{
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::StartUp( InputSystem* theInput, EventSystem* theEventSystem )
{
	m_theInput = theInput;
	m_theEventSystem = theEventSystem;
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::ShutDown()
{
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::EndFrame()
{
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::StartFrame()
{
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if( IsOpen() )
	{
		ProcessInput();
		UpdateCursorBlinkTime( deltaSeconds );
	}
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::UpdateCursorBlinkTime( float deltaSeconds )
{
	if( m_cursorTime >= m_cursorBlinkDelay + m_cursorHideTime )
	{
		m_cursorTime = 0.f;
	}
	m_cursorTime += deltaSeconds;
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::PrintString( const Rgba8& textColor, const std::string& devConsolePrintString )
{
	ColorString newString( textColor, devConsolePrintString );
	m_colorStrings.push_back( newString );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::Render( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const
{
	if( !IsOpen() ) return;

	RenderOutput( renderer, camera, lineHeight, font );

	if( ShowCursor() )
	{
		RenderCursor( renderer, camera, lineHeight, font );
	}

	RenderCurrentInput( renderer, camera, lineHeight, font );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::RenderOutput( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const
{
	std::vector<Vertex_PCU> consoleVerts;
	Vec2 cameraDimensions = camera.GetCameraDimensions();

	int maxNumberOfLines = static_cast<int>( cameraDimensions.y / lineHeight );
	int colorStringLength = static_cast<int>( m_colorStrings.size() );

	int numberOfLinesToPrint = maxNumberOfLines > colorStringLength ? colorStringLength : maxNumberOfLines;
	Vec2 textMins = camera.GetOrthoBottomLeft();
	textMins.y += lineHeight;

	for( int consoleStringIndexFromLast = 0; consoleStringIndexFromLast < numberOfLinesToPrint; ++consoleStringIndexFromLast )
	{
		int consoleStringIndex = colorStringLength - consoleStringIndexFromLast - 1;

		ColorString currentColorString = m_colorStrings[ consoleStringIndex ];

		font->AddVertsForText2D( consoleVerts, textMins, lineHeight, currentColorString.m_text, currentColorString.m_color );

		renderer.BindTexture( font->GetTexture() );
		renderer.BindShader( (Shader*)nullptr );
		renderer.DrawVertexArray( consoleVerts );

		textMins.y += lineHeight;
		consoleVerts.clear();
	}
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::RenderCurrentInput( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const
{
	if( m_currentInput == "" ) return;

	std::vector<Vertex_PCU> inputVerts;
	font->AddVertsForText2D( inputVerts, camera.GetOrthoBottomLeft(), lineHeight, m_currentInput );

	renderer.BindTexture( font->GetTexture() );
	renderer.BindShader( (Shader*)nullptr );
	renderer.DrawVertexArray( inputVerts );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::RenderCursor( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const
{
	float cursorAspect = 0.5f;
	std::vector<Vertex_PCU> cursorVerts;
	std::string beforeString = m_currentInput.substr( 0, m_cursorIndex );
	
	Vec2 inputDimensions = font->GetDimensionsForText2D( lineHeight, beforeString );
	Vec2 cursorDimensions = font->GetDimensionsForText2D( lineHeight, "|" ) * cursorAspect;
	Vec2 cursorPosition = camera.GetOrthoBottomLeft();
	cursorPosition.x -= cursorDimensions.x * 0.4f;
	cursorPosition.x += inputDimensions.x;

	font->AddVertsForText2D( cursorVerts, cursorPosition, lineHeight, "|", Rgba8::WHITE, cursorAspect );

	renderer.BindTexture( font->GetTexture() );
	renderer.BindShader( (Shader*)nullptr );
	renderer.DrawVertexArray( cursorVerts );
}


//---------------------------------------------------------------------------------------------------------
bool DevConsole::ShowCursor() const
{
	if( m_cursorTime >= m_cursorBlinkDelay && m_cursorTime <= m_cursorBlinkDelay + m_cursorHideTime )
	{
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------------------------------------
void DevConsole::ProcessInput()
{
	if( HandleKeyPresses() )
	{
		m_cursorTime = 0.f;
		return;
	}

	char inputCharacter;
	while( m_theInput->PopFromCharacterQueue( &inputCharacter ) )
	{
		AddCharacterToInput( inputCharacter );
		m_cursorTime = 0.f;
	}
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::AddCharacterToInput( char c )
{
	m_currentInput += Stringf( "%c", c );
	m_cursorIndex++;
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::HandleEscapeKey()
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
bool DevConsole::HandleBackspace()
{
	if( m_cursorIndex <= 0 ) return false;

	m_currentInput.erase( m_cursorIndex - 1, 1 );
	m_cursorIndex--;
	return true;
}


//---------------------------------------------------------------------------------------------------------
bool DevConsole::HandleDelete()
{
	m_currentInput.erase( m_cursorIndex, 1 );
	return true;
}


//---------------------------------------------------------------------------------------------------------
bool DevConsole::HandleKeyPresses()
{
	if( m_theInput->WasKeyJustPressed( KEY_CODE_ESC ) )
	{
		HandleEscapeKey();
		return true;
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_ENTER ) )
	{
		SubmitCommand();
		return true;
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_BACKSPACE ) )
	{
		return HandleBackspace();
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_DELETE ) )
	{
		return HandleDelete();
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_LEFT_ARROW ) )
	{
		if( m_cursorIndex > 0 )
		{
			m_cursorIndex--;
		}
		return true;
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_RIGHT_ARROW ) )
	{
		if( m_cursorIndex < m_currentInput.size() )
		{
			m_cursorIndex++;
		}
		return true;
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_UP_ARROW ) )
	{
		int numPreviousCommands = m_previousCommands.size();
		if( m_previousCommandIndex < numPreviousCommands )
		{
			++m_previousCommandIndex;
			m_currentInput = m_previousCommands[ numPreviousCommands - m_previousCommandIndex ];
			return true;
		}
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_DOWN_ARROW ) )
	{
		int numPreviousCommands = m_previousCommands.size();
		if( m_previousCommandIndex > 0 )
		{
			--m_previousCommandIndex;
			if( m_previousCommandIndex == 0 )
			{
				m_currentInput = "";
			}
			else
			{
				m_currentInput = m_previousCommands[ numPreviousCommands - m_previousCommandIndex ];
			}
			return true;
		}
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_HOME ) )
	{
		m_cursorIndex = 0;
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_END ) )
	{
		m_cursorIndex = static_cast<int>( m_currentInput.length() );
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::SubmitCommand()
{
	m_previousCommands.push_back( m_currentInput );

	Strings availableCommands = m_theEventSystem->GetEventNames();
	for( int commandIndex = 0; commandIndex < availableCommands.size(); ++commandIndex )
	{
		std::string command = availableCommands[ commandIndex ];
		if( command == m_currentInput )
		{
			m_theEventSystem->FireEvent( m_currentInput );
		
			m_currentInput = "";
			m_cursorIndex = 0;
			m_previousCommandIndex = 0;
			return;
		}
	}
	
	std::string commandString = m_currentInput + " is not a supported command";
	PrintString( Rgba8::RED, commandString );
	m_currentInput = "";
	m_cursorIndex = 0;
	m_previousCommandIndex = 0;
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::SetIsOpen( bool isOpen )
{
	m_isOpen = isOpen;
	m_currentInput = "";
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::ToggleIsOpen()
{
	SetIsOpen( !m_isOpen );
}

