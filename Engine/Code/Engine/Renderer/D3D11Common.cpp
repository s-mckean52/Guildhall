#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"


//---------------------------------------------------------------------------------------------------------
D3D11_TEXTURE_ADDRESS_MODE ToD3D11TextureAddress( TextureAddressMode textureAddress )
{
	switch( textureAddress )
	{
	case TextureAddressMode::TEXTURE_ADDRESS_BORDER:	return D3D11_TEXTURE_ADDRESS_BORDER;	break;
	case TextureAddressMode::TEXTURE_ADDRESS_CLAMP:		return D3D11_TEXTURE_ADDRESS_CLAMP;		break;
	case TextureAddressMode::TEXTURE_ADDRESS_MIRROR:	return D3D11_TEXTURE_ADDRESS_MIRROR;	break;
	case TextureAddressMode::TEXTURE_ADDRESS_WRAP:		return D3D11_TEXTURE_ADDRESS_WRAP;		break;

	default:
		ERROR_AND_DIE( "Unsupported Texture address" );
		break;
	}
}


//---------------------------------------------------------------------------------------------------------
Vec2 FixUV( const Vec2& uv )
{
	return uv;
}
