#pragma once
#include <string>
#include "Engine/Core/Rgba8.hpp"

struct ColorString
{
public:
	ColorString( const Rgba8& color, std::string text );

public:
	Rgba8 m_color;
	std::string m_text;
};