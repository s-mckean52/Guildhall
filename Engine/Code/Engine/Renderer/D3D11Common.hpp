#pragma once
#if !defined(WIN32_LEAN_AND_MEAN)
	#define WIN32_LEAN_AND_MEAN
#endif

#define RENDER_DEBUG
#define DX_SAFE_RELEASE(obj) if (nullptr != (obj)) { (obj)->Release(); (obj) = nullptr; }
#define BIT_FLAG( b )		( 1 << ( b ) )

//---------------------------------------------------------------------------------------------------------
#define INITGUID
#include <d3d11.h>  // d3d11 specific objects
#include <dxgi.h>   // shared library used across multiple dx graphical interfaces
#include <dxgidebug.h>  // debug utility (mostly used for reporting and analytics)

struct Vec2;

enum class TextureAddressMode
{
	TEXTURE_ADDRESS_BORDER,
	TEXTURE_ADDRESS_CLAMP,
	TEXTURE_ADDRESS_MIRROR,
	TEXTURE_ADDRESS_WRAP,
};

D3D11_TEXTURE_ADDRESS_MODE ToD3D11TextureAddress( TextureAddressMode textureAddress );

Vec2 FixUV( const Vec2& uv );