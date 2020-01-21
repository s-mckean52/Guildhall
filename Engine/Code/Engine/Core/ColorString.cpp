#include "Engine/Core/ColorString.hpp"


//---------------------------------------------------------------------------------------------------------
ColorString::ColorString( const Rgba8& color, std::string text )
	: m_color( color )
	, m_text( text )
{
}