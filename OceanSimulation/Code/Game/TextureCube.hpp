#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
struct Image;

//------------------------------------------------------------------------
enum eTextureCubeSide : uint
{
	TEXCUBE_RIGHT,
	TEXCUBE_LEFT,
	TEXCUBE_TOP,
	TEXCUBE_BOTTOM,
	TEXCUBE_FRONT, // -z
	TEXCUBE_BACK,  // +z

	TEXCUBE_SIDE_COUNT,
};


struct TextureCube : Texture
{
	~TextureCube() {};
	explicit TextureCube( RenderContext* owner ); 

	bool MakeFromImages( Texture const* src );
	bool MakeFromImage( Texture const& src );
	void CreateViews();
};