#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
DevConsole::DevConsole()
{
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::StartUp()
{
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
void DevConsole::PrintString( const Rgba8& textColor, const std::string& devConsolePrintString )
{
	ColorString newString( textColor, devConsolePrintString );
	m_colorStrings.push_back( newString );
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::Render( RenderContext& renderer, const Camera& camera, float lineHeight, BitmapFont* font ) const
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
void DevConsole::SetIsOpen( bool isOpen )
{
	m_isOpen = isOpen;
}


//---------------------------------------------------------------------------------------------------------
void DevConsole::ToggleIsOpen()
{
	m_isOpen = !m_isOpen;
}

