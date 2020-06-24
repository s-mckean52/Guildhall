#pragma once
#include "Game/Tile.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <string>
#include <map>

class SpriteSheet;

class TileDefinition
{
private:
	//Single Constructor
	explicit TileDefinition( const XmlElement& definitionXmlElement );

public:
	AABB2	GetSpriteUVBox()		const { return m_spriteUVBox; }
	Rgba8	GetTint()				const { return m_spriteTint; }
	Rgba8	GetTexelColorForSet()	const { return m_texelColorForSet; }
	bool	IsSolid()				const { return m_isSolid; }

public:
	static void				CreateSpriteSheet( XmlElement const& element );
	static void				CreateTileDefinition( XmlElement const& element );
	static void				InitializeTileDefinitions();
	static TileDefinition*	GetTileDefByName( std::string const& tileDefName );
	static TileDefinition*	GetTileDefWithSetColor( const Rgba8& texelColor );

	static std::map< std::string, TileDefinition* >		s_tileDefinitions;
	static std::map< std::string, SpriteSheet* >		s_spriteSheets;

private:
	std::string		m_name				= "Default";
	SpriteSheet*	m_spriteSheet		= nullptr;
	AABB2			m_spriteUVBox		= AABB2();
	Rgba8			m_spriteTint		= Rgba8();
	Rgba8			m_texelColorForSet	= Rgba8();
	bool			m_isSolid			= false;
};
