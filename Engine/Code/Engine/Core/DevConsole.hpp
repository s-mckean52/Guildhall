#pragma once
#include <string>
#include <vector>
#include "Engine/Core/ColorString.hpp"

struct	Rgba8;
class	Camera;
class	RenderContext;
class	BitmapFont;
class	InputSystem;
class	EventSystem;


class DevConsole
{
public:
	DevConsole();
	
	void StartUp( InputSystem* theInput, EventSystem* theEventSystem );
	void ShutDown();
	void EndFrame();
	void StartFrame();
	void Update( float deltaSeconds );

	void PrintString( const Rgba8& textColor, const std::string& devConsolePrintString );

	void Render( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;
	void RenderOutput( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;
	void RenderCurrentInput( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;
	void RenderCursor( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;

	void ProcessInput();
	void AddCharacterToInput( char c );
	void HandleEscapeKey();
	bool HandleBackspace();
	bool HandleDelete();
	bool HandleKeyPresses();
	void SubmitCommand();
	
	void SetIsOpen( bool isOpen );
	void ToggleIsOpen();
	bool IsOpen() const { return m_isOpen; };

private:
	int m_cursorIndex = 0;

	bool m_isOpen = false;
	InputSystem* m_theInput = nullptr;
	EventSystem* m_theEventSystem = nullptr;


	std::string m_currentInput = "";
	std::vector< ColorString > m_colorStrings;
};