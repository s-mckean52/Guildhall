//#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Rgba8.hpp"


//---------------------------------------------------------------------------------------------------------
Image::Image( const char* imageFilePath )
	: m_imageFilePath( imageFilePath )
{
	int numComponents = 0;
	int numComponentsRequested = 0;

	stbi_set_flip_vertically_on_load( 1 ); //Start position is Bottom Left;
	unsigned char* imageData = stbi_load( m_imageFilePath.c_str(), &m_dimensions.x, &m_dimensions.y, &numComponents, numComponentsRequested );

	// Check if the load was successful
	GUARANTEE_OR_DIE( imageData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );
	GUARANTEE_OR_DIE( numComponents >= 3 && numComponents <= 4 && m_dimensions.x > 0 && m_dimensions.y > 0, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, m_dimensions.x, m_dimensions.y ) );

	int imageDataSize = ( m_dimensions.x * m_dimensions.y ) * numComponents;

	for( int texelIndex = 0; texelIndex < imageDataSize; texelIndex += numComponents )
	{
		unsigned char a = 255;
		unsigned char r = imageData[ texelIndex ];
		unsigned char g = imageData[ texelIndex + 1 ];
		unsigned char b = imageData[ texelIndex + 2 ];

		if( numComponents == 4 )
		{
			a = imageData[ texelIndex + 3 ];
		}

		Rgba8 newTexel( r, g, b, a );
		m_rgbaTexles.push_back( newTexel );
	}

	stbi_image_free( imageData );
}


//---------------------------------------------------------------------------------------------------------
const std::string& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}


//---------------------------------------------------------------------------------------------------------
IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}


//---------------------------------------------------------------------------------------------------------
Rgba8 Image::GetTexelColor( int texelX, int texelY ) const
{
	int texelIndex = ( texelY * m_dimensions.x ) + texelX;
	return m_rgbaTexles[ texelIndex ];
}


//---------------------------------------------------------------------------------------------------------
Rgba8 Image::GetTexelColor( IntVec2 texelCoords ) const
{
	return GetTexelColor( texelCoords.x, texelCoords.y );
}


//---------------------------------------------------------------------------------------------------------
void Image::SetTexelColor( int texelX, int texelY, const Rgba8& newTexelColor )
{
	int texelIndex = ( texelY * m_dimensions.x ) + texelX;
	m_rgbaTexles[ texelIndex ] = newTexelColor;
}


//---------------------------------------------------------------------------------------------------------
void Image::SetTexelColor( IntVec2 texelCoords, const Rgba8& newTexelColor )
{
	SetTexelColor( texelCoords.x, texelCoords.y, newTexelColor );
}
