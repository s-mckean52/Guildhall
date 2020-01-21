#include <windows.h>

#include <gl/gl.h>					// Include basic OpenGL constants and function declarations
#pragma comment( lib, "opengl32" )	// Link in the OpenGL32.lib static library

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"


//---------------------------------------------------------------------------------------------------------
void RenderContext::StartUp()
{
	glEnable( GL_BLEND );
	SetBlendMode( BlendMode::ALPHA );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BeginFrame()
{

}


//---------------------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{

}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ShutDown()
{

}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetBlendMode( BlendMode blendMode )
{
	switch( blendMode )
	{
	case BlendMode::ALPHA:
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		break;

	case BlendMode::ADDITIVE:
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
		break;

	default:
		ERROR_AND_DIE( Stringf( "Unkown or unsupported blend mode #%i", blendMode ) );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ClearScreen( const Rgba8& clearColor )
{
	glClearColor( (float)clearColor.r / 255.f, (float)clearColor.g / 255.f, (float)clearColor.b / 255.f, 1.f);		// Note; glClearColor takes colors as floats in [0,1], not bytes in [0,255]
	glClear( GL_COLOR_BUFFER_BIT );																				// ALWAYS clear the screen at the top of each frame's Render()!
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BeginCamera( const Camera& camera )
{
	Vec2 bottomLeft = camera.GetOrthoBottomLeft();
	Vec2 topRight = camera.GetOrthoTopRight();

	glLoadIdentity();

	glOrtho( bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, 0.f, 1.f );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::EndCamera( const Camera& camera )
{
	UNUSED( camera );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( int numVerticies, const Vertex_PCU* verticies )
{
	glBegin( GL_TRIANGLES );
	{
		for( int vertIndex = 0; vertIndex < numVerticies; vertIndex++ )
		{
			Rgba8 vertexColor = verticies[ vertIndex ].m_color;
			Vec3 vertexPos = verticies[ vertIndex ].m_position;
			Vec2 vertexUVs = verticies[ vertIndex ].m_uvTexCoords;

			glTexCoord2f( vertexUVs.x, vertexUVs.y );
			glColor4ub( vertexColor.r, vertexColor.g, vertexColor.b, vertexColor.a );
			glVertex3f( vertexPos.x, vertexPos.y, vertexPos.z );
		}
	}
	glEnd();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( const std::vector<Vertex_PCU>& vertexArray )
{
	DrawVertexArray( static_cast<int>(vertexArray.size()), &vertexArray[ 0 ] );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::CreateTextureFromFile( const char* imageFilePath )
{
	unsigned int textureID = 0;
	int imageTexelSizeX = 0;
	int imageTexelSizeY = 0;
	int numComponents = 0; 
	int numComponentsRequested = 0;

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 );
	unsigned char* imageData = stbi_load( imageFilePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );

	// Check if the load was successful
	GUARANTEE_OR_DIE( imageData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );
	GUARANTEE_OR_DIE( numComponents >= 3 && numComponents <= 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, imageTexelSizeX, imageTexelSizeY ) );

	// Enable OpenGL texturing
	glEnable( GL_TEXTURE_2D );

	// Tell OpenGL that our pixel data is single-byte aligned
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	// Ask OpenGL for an unused texName (ID number) to use for this texture
	glGenTextures( 1, (GLuint*)&textureID );

	// Tell OpenGL to bind (set) this as the currently active texture
	glBindTexture( GL_TEXTURE_2D, textureID );

	// Set texture clamp vs. wrap (repeat) default settings
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // GL_CLAMP or GL_REPEAT
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // GL_CLAMP or GL_REPEAT

	// Set magnification (texel > pixel) and minification (texel < pixel) filters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR

	// Pick the appropriate OpenGL format (RGB or RGBA) for this texel data
	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
	if( numComponents == 3 )
	{
		bufferFormat = GL_RGB;
	}
	GLenum internalFormat = bufferFormat; // the format we want the texture to be on the card; technically allows us to translate into a different texture format as we upload to OpenGL

	// Upload the image texel data (raw pixels bytes) to OpenGL under this textureID
	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
		GL_TEXTURE_2D,		// Creating this as a 2d texture
		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
		imageTexelSizeX,	// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,11], and B is the border thickness [0,1]
		imageTexelSizeY,	// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,11], and B is the border thickness [0,1]
		0,					// Border size, in texels (must be 0 or 1, recommend 0)
		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color channel/component)
		imageData );		// Address of the actual pixel data bytes/buffer in system memory

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free( imageData );

	m_loadedTextures.push_back( new Texture( imageFilePath, textureID, IntVec2( imageTexelSizeX, imageTexelSizeY ), numComponents ) );
}


//---------------------------------------------------------------------------------------------------------
Texture* RenderContext::CreateOrGetTextureFromFile( const char* imageFilePath )
{
	Texture* loadedTexture;
	for( int loadedTexturesIndex = 0; loadedTexturesIndex < m_loadedTextures.size(); ++loadedTexturesIndex )
	{
		loadedTexture = m_loadedTextures[ loadedTexturesIndex ];
		if( imageFilePath == loadedTexture->GetImageFilePath() )
		{
			return loadedTexture;
		}
	}
	CreateTextureFromFile( imageFilePath );
	return m_loadedTextures[ m_loadedTextures.size() - 1 ];
}


//---------------------------------------------------------------------------------------------------------
BitmapFont* RenderContext::CreateOrGetBitmapFontFromFile( const char* imageFilePath )
{
	BitmapFont* loadedBitFont;
	for( int loadedFontIndex = 0; loadedFontIndex < m_loadedFonts.size(); ++loadedFontIndex )
	{
		loadedBitFont = m_loadedFonts[ loadedFontIndex ];
		if( imageFilePath == loadedBitFont->m_fontName )
		{
			return loadedBitFont;
		}
	}
	CreateBitmapFontFromFile( imageFilePath );
	return m_loadedFonts[ m_loadedFonts.size() - 1 ];
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BindTexture( const Texture* texture )
{
	if( texture )
	{
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, texture->GetTextureID() );
	}
	else
	{
		glDisable( GL_TEXTURE_2D );
	}
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::CreateBitmapFontFromFile( const char* fontFilePath )
{
	std::string fontImagePath = Stringf( "%s.png", fontFilePath );
	Texture* fontTexture = CreateOrGetTextureFromFile( fontImagePath.c_str() );
	BitmapFont* newFont = new BitmapFont( fontFilePath, fontTexture );
	m_loadedFonts.push_back( newFont );
}