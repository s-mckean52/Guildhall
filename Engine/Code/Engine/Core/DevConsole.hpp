#pragma once
#include <string>
#include <vector>
#include "Engine/Core/ColorString.hpp"

struct	Rgba8;
class	Camera;
class	RenderContext;
class	BitmapFont;

class DevConsole
{
public:
	DevConsole();
	
	void StartUp();
	void ShutDown();
	void EndFrame();
	void StartFrame();

	void PrintString( const Rgba8& textColor, const std::string& devConsolePrintString );

	void Render( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const;

	void SetIsOpen( bool isOpen );
	void ToggleIsOpen();
	bool IsOpen() const { return m_isOpen; };

private:
	bool m_isOpen = false;
	std::vector< ColorString > m_colorStrings;
};