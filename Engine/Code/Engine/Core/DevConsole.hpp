#pragma once
#include "Engine/Core/ColorString.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include <vector>

class	Camera;
class	RenderContext;
class	BitmapFont;
class	InputSystem;
class	EventSystem;
class	Clock;

enum Direction
{
	LEFT,
	RIGHT,
};

class DevConsole
{
public:
	DevConsole();
	
	void StartUp( InputSystem* theInput, EventSystem* theEventSystem );
	void ShutDown();
	void EndFrame();
	void StartFrame();

	void Update();
	void UpdateCursorBlinkTime( float deltaSeconds );

	void PrintString( const Rgba8& textColor, const std::string& devConsolePrintString );

	void Render( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;
	void RenderBackground( RenderContext& renderer, const Camera& camera ) const;
	void RenderOutput( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;
	void RenderCurrentInput( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;
	void RenderCursor( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;
	void RenderSelection( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;
	bool ShowCursor() const;

	void ProcessInput();
	void AddCharacterToInput( char c );

	bool MoveCursor( Direction direction, bool isShiftHeld, bool isCtrlHeld );
	void GetSelectionRange( int& out_start, int& out_end ) const;
	void SelectText( Direction direction );
	void EndSelection( Direction direction );
	void DeleteSelection();
	void JumpWord( Direction direction );
	
	bool HandleKeyPresses();
	void HandleEscapeKey();
	bool HandleBackspace();
	bool HandleDelete();
	void SubmitCommand();
	void ResetInput();

	void HandleCut();
	void HandleCopy();
	void HandlePaste();

	void ScrollPreviousCommands( int numCommands );

	void SetIsOpen( bool isOpen );
	void ToggleIsOpen();
	bool IsOpen() const { return m_isOpen; };

private:
	Clock* m_clock = nullptr;

	int m_cursorPosition = 0;
	int m_selectionOffset = 0;

	int m_previousCommandIndex = 0;
	float m_cursorTime = 0.f;
	float m_cursorHideTime = 0.2f;
	float m_cursorBlinkDelay = 0.65f;

	Rgba8 m_selectionColor = Rgba8( 0, 0, 200, 80 );
	Rgba8 m_backgroundColor = Rgba8( 50, 50, 50, 175 );

	bool m_isOpen = false;
	InputSystem* m_theInput = nullptr;
	EventSystem* m_theEventSystem = nullptr;


	std::string m_currentInput = "";
	std::vector< ColorString > m_colorStrings;
	std::vector< std::string > m_previousCommands;
};