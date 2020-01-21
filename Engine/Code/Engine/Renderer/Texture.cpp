#include "Engine/Renderer/Texture.hpp"


//---------------------------------------------------------------------------------------------------------
Texture::Texture( const char* imageFilePath, int textureID, IntVec2 imageTexelSize, int numComponents )
	: m_imageFilePath( imageFilePath )
	, m_textureID( textureID )
	, m_imageTexelSize( imageTexelSize )
	, m_numComponents( numComponents )
{
}


//---------------------------------------------------------------------------------------------------------
float Texture::GetAspect() const
{
	return static_cast<float>( m_imageTexelSize.x ) / static_cast<float>( m_imageTexelSize.y );
}

