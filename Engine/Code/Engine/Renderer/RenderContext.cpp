//---------------------------------------------------------------------------------------------------------
//		THIRD PARTY INCLUDES
//---------------------------------------------------------------------------------------------------------

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"

#include "Engine/Renderer/D3D11Common.hpp"

#pragma comment( lib, "d3d11.lib" )         // needed a01
#pragma comment( lib, "dxgi.lib" )          // needed a01
#pragma comment( lib, "d3dcompiler.lib" )   // needed when we get to shaders

//---------------------------------------------------------------------------------------------------------

#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Platform/Window.hpp"
#include "Engine/Renderer/SwapChain.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/Shader.hpp"


//---------------------------------------------------------------------------------------------------------
void RenderContext::StartUp( Window* theWindow )
{
/*	SetBlendMode( BlendMode::ALPHA );*/

	IDXGISwapChain* swapchain;

	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined(RENDER_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	memset( &swapchainDesc, 0, sizeof( swapchainDesc ) );
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapchainDesc.Flags = 0;

	HWND hwnd = (HWND) theWindow->m_hwnd;
	swapchainDesc.OutputWindow = hwnd;
	swapchainDesc.SampleDesc.Count = 1;

	swapchainDesc.Windowed = TRUE;
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferDesc.Width = theWindow->GetClientWidth();
	swapchainDesc.BufferDesc.Height = theWindow->GetClientHeight();



	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags, //controls the type of device we make
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&swapchainDesc,
		&swapchain,
		&m_device,
		nullptr,
		&m_context );

	GUARANTEE_OR_DIE( SUCCEEDED( result ), "Failed to create rendering pipeline" );

	m_swapchain = new SwapChain( this, swapchain );
	m_currentShader = new Shader();
	m_currentShader->CreateFromFile( "Data/Shaders/triangle.hlsl" );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BeginFrame()
{

}


//---------------------------------------------------------------------------------------------------------
void RenderContext::EndFrame()
{
	m_swapchain->Present();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ShutDown()
{
	delete m_swapchain;
	m_swapchain = nullptr;

	DX_SAFE_RELEASE( m_device );
	DX_SAFE_RELEASE( m_context );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::SetBlendMode( BlendMode blendMode )
{
	UNUSED( blendMode );
// 	switch( blendMode )
// 	{
// 	case BlendMode::ALPHA:
// 		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
// 		break;
// 
// 	case BlendMode::ADDITIVE:
// 		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
// 		break;
// 
// 	default:
// 		ERROR_AND_DIE( Stringf( "Unkown or unsupported blend mode #%i", blendMode ) );
// 		break;
// 	}
	UNIMPLEMENTED_MSG( "OpenGl Blend mode stuff" );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::ClearScreen( const Rgba8& clearColor )
{
	float clearFloats [4];
	clearFloats[0] = (float)clearColor.r / 255.0f;
	clearFloats[1] = (float)clearColor.g / 255.0f;
	clearFloats[2] = (float)clearColor.b / 255.0f;
	clearFloats[3] = (float)clearColor.a / 255.0f;

	Texture* backbuffer = m_swapchain->GetBackBuffer();
	TextureView* backbuffer_rtv = backbuffer->GetRenderTargetView();

	ID3D11RenderTargetView* rtv = backbuffer_rtv->GetRTVHandle();
	m_context->ClearRenderTargetView( rtv, clearFloats );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::BeginCamera( const Camera& camera )
{
	ClearScreen( camera.GetClearColor() );
	Vec2 bottomLeft = camera.GetOrthoBottomLeft();
	Vec2 topRight = camera.GetOrthoTopRight();
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::EndCamera( const Camera& camera )
{
	UNUSED( camera );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::Draw( int numVertices, int vertexOffset )
{
	m_context->Draw( numVertices, vertexOffset );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( int numVerticies, const Vertex_PCU* verticies )
{
	UNUSED( verticies );
	UNUSED( numVerticies );
// 	glBegin( GL_TRIANGLES );
// 	{
// 		for( int vertIndex = 0; vertIndex < numVerticies; vertIndex++ )
// 		{
// 			Rgba8 vertexColor = verticies[ vertIndex ].m_color;
// 			Vec3 vertexPos = verticies[ vertIndex ].m_position;
// 			Vec2 vertexUVs = verticies[ vertIndex ].m_uvTexCoords;
// 
// 			glTexCoord2f( vertexUVs.x, vertexUVs.y );
// 			glColor4ub( vertexColor.r, vertexColor.g, vertexColor.b, vertexColor.a );
// 			glVertex3f( vertexPos.x, vertexPos.y, vertexPos.z );
// 		}
// 	}
// 	glEnd();
	UNIMPLEMENTED_MSG( "OpenGl draw vertex" );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::DrawVertexArray( const std::vector<Vertex_PCU>& vertexArray )
{
	DrawVertexArray( static_cast<int>(vertexArray.size()), &vertexArray[ 0 ] );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::CreateTextureFromFile( const char* imageFilePath )
{
	UNUSED( imageFilePath );
// 	unsigned int textureID = 0;
// 	int imageTexelSizeX = 0;
// 	int imageTexelSizeY = 0;
// 	int numComponents = 0; 
// 	int numComponentsRequested = 0;
// 
// 	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
// 	stbi_set_flip_vertically_on_load( 1 );
// 	unsigned char* imageData = stbi_load( imageFilePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );
// 
// 	// Check if the load was successful
// 	GUARANTEE_OR_DIE( imageData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );
// 	GUARANTEE_OR_DIE( numComponents >= 3 && numComponents <= 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, imageTexelSizeX, imageTexelSizeY ) );
// 
// 	// Enable OpenGL texturing
// 	glEnable( GL_TEXTURE_2D );
// 
// 	// Tell OpenGL that our pixel data is single-byte aligned
// 	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
// 
// 	// Ask OpenGL for an unused texName (ID number) to use for this texture
// 	glGenTextures( 1, (GLuint*)&textureID );
// 
// 	// Tell OpenGL to bind (set) this as the currently active texture
// 	glBindTexture( GL_TEXTURE_2D, textureID );
// 
// 	// Set texture clamp vs. wrap (repeat) default settings
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ); // GL_CLAMP or GL_REPEAT
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); // GL_CLAMP or GL_REPEAT
// 
// 	// Set magnification (texel > pixel) and minification (texel < pixel) filters
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ); // one of: GL_NEAREST, GL_LINEAR
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); // one of: GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR
// 
// 	// Pick the appropriate OpenGL format (RGB or RGBA) for this texel data
// 	GLenum bufferFormat = GL_RGBA; // the format our source pixel data is in; any of: GL_RGB, GL_RGBA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, ...
// 	if( numComponents == 3 )
// 	{
// 		bufferFormat = GL_RGB;
// 	}
// 	GLenum internalFormat = bufferFormat; // the format we want the texture to be on the card; technically allows us to translate into a different texture format as we upload to OpenGL
// 
// 	// Upload the image texel data (raw pixels bytes) to OpenGL under this textureID
// 	glTexImage2D(			// Upload this pixel data to our new OpenGL texture
// 		GL_TEXTURE_2D,		// Creating this as a 2d texture
// 		0,					// Which mipmap level to use as the "root" (0 = the highest-quality, full-res image), if mipmaps are enabled
// 		internalFormat,		// Type of texel format we want OpenGL to use for this texture internally on the video card
// 		imageTexelSizeX,	// Texel-width of image; for maximum compatibility, use 2^N + 2^B, where N is some integer in the range [3,11], and B is the border thickness [0,1]
// 		imageTexelSizeY,	// Texel-height of image; for maximum compatibility, use 2^M + 2^B, where M is some integer in the range [3,11], and B is the border thickness [0,1]
// 		0,					// Border size, in texels (must be 0 or 1, recommend 0)
// 		bufferFormat,		// Pixel format describing the composition of the pixel data in buffer
// 		GL_UNSIGNED_BYTE,	// Pixel color components are unsigned bytes (one byte per color channel/component)
// 		imageData );		// Address of the actual pixel data bytes/buffer in system memory
// 
// 	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
// 	stbi_image_free( imageData );
// 
// 	m_loadedTextures.push_back( new Texture( imageFilePath, textureID, IntVec2( imageTexelSizeX, imageTexelSizeY ), numComponents ) );
	UNIMPLEMENTED_MSG( "OpenGL load texture" );
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
	UNUSED( texture );
// 	if( texture )
// 	{
// 		glEnable( GL_TEXTURE_2D );
// 		glBindTexture( GL_TEXTURE_2D, texture->GetTextureID() );
// 	}
// 	else
// 	{
// 		glDisable( GL_TEXTURE_2D );
// 	}
	UNIMPLEMENTED_MSG( "OpenGl Bind Texture" );
}


//---------------------------------------------------------------------------------------------------------
void RenderContext::CreateBitmapFontFromFile( const char* fontFilePath )
{
	std::string fontImagePath = Stringf( "%s.png", fontFilePath );
	Texture* fontTexture = CreateOrGetTextureFromFile( fontImagePath.c_str() );
	BitmapFont* newFont = new BitmapFont( fontFilePath, fontTexture );
	m_loadedFonts.push_back( newFont );
}