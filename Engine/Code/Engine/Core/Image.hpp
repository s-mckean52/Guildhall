#pragma once
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"

struct Rgba8;

struct Image
{
public:
	Image( const char* imageFilePath );
	const std::string&	GetImageFilePath() const;
	IntVec2				GetDimensions() const;
	Rgba8				GetTexelColor( int texelX, int texelY ) const;
	Rgba8				GetTexelColor( IntVec2 texelCoords ) const;
	void				SetTexelColor( int texelX, int texelY, const Rgba8& newTexelColor );
	void				SetTexelColor( IntVec2 texelCoords, const Rgba8& newTexelColor );

private:
	std::string				m_imageFilePath;
	IntVec2					m_dimensions = IntVec2( 0, 0 );
	std::vector< Rgba8 >	m_rgbaTexles;
};