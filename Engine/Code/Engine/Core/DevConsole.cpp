#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/AABB2.hpp"


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

	RenderBackground( renderer, camera );
	RenderOutput( renderer, camera, lineHeight, font );
	RenderCursor( renderer, camera, lineHeight, font );
	RenderCurrentInput( renderer, camera, lineHeight, font );
	RenderSelection( renderer, camera, lineHeight, font );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::RenderBackground( RenderContext& renderer, const Camera& camera ) const
{
	AABB2 backgroundQuad;
	std::vector<Vertex_PCU> backgroundVerts;

	Vec3 orthoMins = camera.GetOrthoBottomLeft();
	Vec3 orthoMaxes = camera.GetOrthoTopRight();

	backgroundQuad.mins = Vec2( orthoMins.x, orthoMins.y );
	backgroundQuad.maxes = Vec2( orthoMaxes.x, orthoMaxes.y );

	AppendVertsForAABB2D( backgroundVerts, backgroundQuad, m_backgroundColor );

	renderer.BindTexture( (Texture*)nullptr );
	renderer.BindShader( (Shader*)nullptr );
	renderer.DrawVertexArray( backgroundVerts );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::RenderOutput( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const
{
	std::vector<Vertex_PCU> consoleVerts;
	Vec2 cameraDimensions = camera.GetCameraDimensions();

	int maxNumberOfLines = static_cast<int>( cameraDimensions.y / lineHeight );
	int colorStringLength = static_cast<int>( m_colorStrings.size() );

	int numberOfLinesToPrint = maxNumberOfLines > colorStringLength ? colorStringLength : maxNumberOfLines;
	Vec3 textMins = camera.GetOrthoBottomLeft();
	textMins.y += lineHeight;

	for( int consoleStringIndexFromLast = 0; consoleStringIndexFromLast < numberOfLinesToPrint; ++consoleStringIndexFromLast )
	{
		int consoleStringIndex = colorStringLength - consoleStringIndexFromLast - 1;

		ColorString currentColorString = m_colorStrings[ consoleStringIndex ];

		font->AddVertsForText2D( consoleVerts, Vec2( textMins.x, textMins.y ), lineHeight, currentColorString.m_text, currentColorString.m_color );

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
	Vec3 textMins = camera.GetOrthoBottomLeft();
	font->AddVertsForText2D( inputVerts, Vec2( textMins.x, textMins.y ), lineHeight, m_currentInput );

	renderer.BindTexture( font->GetTexture() );
	renderer.BindShader( (Shader*)nullptr );
	renderer.DrawVertexArray( inputVerts );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::RenderCursor( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const
{
	if( !ShowCursor() ) return;

	float cursorAspect = 0.5f;
	std::vector<Vertex_PCU> cursorVerts;
	std::string beforeString = m_currentInput.substr( 0, m_cursorPosition + m_selectionOffset );
	
	Vec2 inputDimensions = font->GetDimensionsForText2D( lineHeight, beforeString );
	Vec2 cursorDimensions = font->GetDimensionsForText2D( lineHeight, "|" ) * cursorAspect;
	Vec3 cursorPosition = camera.GetOrthoBottomLeft();
	cursorPosition.x -= cursorDimensions.x * 0.4f;
	cursorPosition.x += inputDimensions.x;

	font->AddVertsForText2D( cursorVerts, Vec2( cursorPosition.x, cursorPosition.y ), lineHeight, "|", Rgba8::WHITE, cursorAspect );

	renderer.BindTexture( font->GetTexture() );
	renderer.BindShader( (Shader*)nullptr );
	renderer.DrawVertexArray( cursorVerts );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::RenderSelection( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const
{
	if( m_selectionOffset == 0 ) return;

	AABB2 selectionBox;
	std::vector<Vertex_PCU> selectionVerts;
	int startPos = 0;
	int endPos = 0;

	GetSelectionRange( startPos, endPos );

	int range = endPos - startPos;

	std::string beforeString = m_currentInput.substr( 0, startPos );
	std::string selectedString = m_currentInput.substr( startPos, range );

	Vec2 beforeDimensions = font->GetDimensionsForText2D( lineHeight, beforeString );
	Vec2 selectionDimensions = font->GetDimensionsForText2D( lineHeight, selectedString );

	Vec3 orthoMins = camera.GetOrthoBottomLeft();
	selectionBox.mins = Vec2( orthoMins.x, orthoMins.y );
	selectionBox.mins.x += beforeDimensions.x;
	selectionBox.maxes = selectionBox.mins + selectionDimensions;

	AppendVertsForAABB2D( selectionVerts, selectionBox, m_selectionColor );

	renderer.BindTexture( (Texture*)nullptr );
	renderer.BindShader( (Shader*)nullptr );
	renderer.DrawVertexArray( selectionVerts );
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
	if( c == KEY_CODE_BACKSPACE )
	{
		HandleBackspace();
	}
	else if( c == KEY_CODE_COPY )
	{
		HandleCopy();
	}
	else if( c == KEY_CODE_PASTE )
	{
		HandlePaste();
	}
	else if( c == KEY_CODE_CUT )
	{
		HandleCut();
	}
	else
	{
		if( m_selectionOffset != 0 )
		{
			DeleteSelection();
		}
		m_currentInput.insert( m_cursorPosition, 1, c );
		++m_cursorPosition;
	}
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::HandleEscapeKey()
{
	if( m_currentInput != "" )
	{
		ResetInput();
	}
	else
	{
		g_theConsole->ToggleIsOpen();
	}
}


//---------------------------------------------------------------------------------------------------------
bool DevConsole::HandleBackspace()
{
	if( m_selectionOffset == 0 && m_cursorPosition > 0 )
	{
		m_currentInput.erase( m_cursorPosition - 1, 1 );
		--m_cursorPosition;
	}
	else
	{
		DeleteSelection();
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
bool DevConsole::HandleDelete()
{
	if( m_selectionOffset == 0 )
	{
		m_currentInput.erase( m_cursorPosition, 1 );
	}
	else
	{
		DeleteSelection();
	}
	return true;
}


//---------------------------------------------------------------------------------------------------------
bool DevConsole::HandleKeyPresses()
{
	bool ctrlHeld = false;
	bool shiftHeld = false;

	if( m_theInput->IsKeyPressed( KEY_CODE_CTRL ) )
	{
		ctrlHeld = true;
	}

	if( m_theInput->IsKeyPressed( KEY_CODE_SHIFT ) )
	{
		shiftHeld = true;
	}

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
		return MoveCursor( LEFT, shiftHeld, ctrlHeld );
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_RIGHT_ARROW ) )
	{
		return MoveCursor( RIGHT, shiftHeld, ctrlHeld );
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_UP_ARROW ) )
	{
		int numPreviousCommands = static_cast<int>( m_previousCommands.size() );
		if( m_previousCommandIndex < numPreviousCommands )
		{
			++m_previousCommandIndex;
			ScrollPreviousCommands( numPreviousCommands );
			return true;
		}
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_DOWN_ARROW ) )
	{
		int numPreviousCommands = static_cast<int>( m_previousCommands.size() );
		if( m_previousCommandIndex > 0 )
		{
			--m_previousCommandIndex;
			if( m_previousCommandIndex == 0 )
			{
				ResetInput();
			}
			else
			{
				ScrollPreviousCommands( numPreviousCommands );
			}
			return true;
		}
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_HOME ) )
	{
		m_selectionOffset = 0;
		m_cursorPosition = 0;
	}

	if( m_theInput->WasKeyJustPressed( KEY_CODE_END ) )
	{
		m_selectionOffset = 0;
		m_cursorPosition = static_cast<int>( m_currentInput.length() );
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::SelectText( Direction direction )
{
	switch( direction )
	{
	case LEFT:
		--m_selectionOffset;
		break;
	case RIGHT:
		++m_selectionOffset;
		break;
	}

	int currentCursorPosition = m_cursorPosition + m_selectionOffset;
	if( currentCursorPosition < 0 )
	{
		++m_selectionOffset;
	}
	else if( currentCursorPosition > m_currentInput.size() )
	{
		--m_selectionOffset;
	}
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
			ResetInput();
			return;
		}
	}
	
	std::string commandString = m_currentInput + " is not a supported command";
	PrintString( Rgba8::RED, commandString );
	ResetInput();
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::SetIsOpen( bool isOpen )
{
	m_isOpen = isOpen;
	ResetInput();
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::ToggleIsOpen()
{
	SetIsOpen( !m_isOpen );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::ResetInput()
{
	m_currentInput = "";
	m_cursorPosition = 0;
	m_selectionOffset = 0;
	m_previousCommandIndex = 0;
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::HandleCut()
{
	HandleCopy();
	DeleteSelection();
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::HandleCopy()
{
	if( m_selectionOffset == 0 ) return;

	int startPos;
	int endPos;
	std::string copiedString = "";

	GetSelectionRange( startPos, endPos );
	int range = endPos - startPos;

	copiedString = m_currentInput.substr( startPos, range );
	m_theInput->AddStringToClipboard( copiedString );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::HandlePaste()
{
	if( m_selectionOffset != 0 )
	{
		DeleteSelection();
	}

	std::string stringToPaste = m_theInput->GetStringFromClipboard();
	m_currentInput.insert( m_cursorPosition, stringToPaste );
	m_cursorPosition += static_cast<int>( stringToPaste.size() );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::ScrollPreviousCommands( int numCommands )
{
	int previousCommandFromBack = numCommands - m_previousCommandIndex;
	m_currentInput = m_previousCommands[ previousCommandFromBack ];
	m_cursorPosition = static_cast<int>(m_currentInput.size());
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::DeleteSelection()
{
	int startPos;
	int endPos;

	GetSelectionRange( startPos, endPos );

	int range = endPos - startPos;

	m_currentInput.erase( static_cast<size_t>( startPos ), range );
	m_cursorPosition = startPos;
	m_selectionOffset = 0;
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::JumpWord( Direction direction )
{
	bool isWhiteSpace = false;
	std::vector<int> wordStartIndexes;
	wordStartIndexes.push_back( 0 );
	for( int inputIndex = 0; inputIndex < m_currentInput.size(); ++inputIndex )
	{
		if( isspace( m_currentInput[ inputIndex ] ) )
		{
			isWhiteSpace = true;
		}
		else
		{
			if( isWhiteSpace )
			{
				wordStartIndexes.push_back( inputIndex );
				isWhiteSpace = false;
			}
		}
	}
	wordStartIndexes.push_back( static_cast<int>( m_currentInput.size() ) );
	
	for( int wordStartIndex = 0; wordStartIndex < wordStartIndexes.size(); ++wordStartIndex )
	{
		int currentWordStart = wordStartIndexes[ wordStartIndex ];
		if( direction == LEFT && currentWordStart >= m_cursorPosition )
		{
			if( m_cursorPosition == 0 ) break;
			m_cursorPosition = wordStartIndexes[ wordStartIndex - 1 ];
			break;
		}
		else if( direction == RIGHT && currentWordStart > m_cursorPosition )
		{
			m_cursorPosition = currentWordStart;
			break;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::GetSelectionRange( int& out_start, int& out_end ) const
{
	if( m_selectionOffset < 0 )
	{
		out_start = m_cursorPosition + m_selectionOffset;
		out_end = m_cursorPosition;
	}
	else
	{
		out_start = m_cursorPosition;
		out_end = m_cursorPosition + m_selectionOffset;
	}
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::EndSelection( Direction direction )
{
	if( m_selectionOffset == 0 ) return;

	int startPos = 0;
	int endPos = 0;

	GetSelectionRange( startPos, endPos );

	switch( direction )
	{
	case LEFT:
		m_cursorPosition = startPos;
		break;
	case RIGHT:
		m_cursorPosition = endPos;
		break;
	default:
		break;
	}

	m_selectionOffset = 0;
}


//---------------------------------------------------------------------------------------------------------
bool DevConsole::MoveCursor( Direction directionToMove, bool isShiftHeld, bool isCtrlHeld )
{
	if( isShiftHeld )
	{
		SelectText( directionToMove );
		return true;
	}
	else if( m_selectionOffset != 0 )
	{
		EndSelection( directionToMove );
		return true;
	}
	
	if( isCtrlHeld )
	{
		JumpWord( directionToMove );
		return true;
	}

	if( m_cursorPosition > 0 && directionToMove == LEFT)
	{
		--m_cursorPosition;
	}
	else if( m_cursorPosition < m_currentInput.size() && directionToMove == RIGHT )
	{
		++m_cursorPosition;
	}
	return true;
}