#pragma once
#include "Game/Tile.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include <map>
#include <string>

class TileDefinition
{
private:
	//Single Constructor
	explicit TileDefinition( const XmlElement& definitionXmlElement );

public:
	AABB2	GetSpriteUVBox()		const { return m_spriteUVBox; }
	Rgba8	GetTint()				const { return m_spriteTint; }
	Rgba8	GetTexelColorForSet()	const { return m_texelColorForSet; }
	bool	DoesAllowWalking()		const { return m_allowWalking; }
	bool	DoesAllowSwimming()		const { return m_allowSwimming; }
	bool	DoesAllowFlying()		const { return m_allowFlying; }

public:
	static void				InitializeTileDefinitions();
	static TileDefinition*	GetTileDefWithSetColor( const Rgba8& texelColor );
	static std::map< std::string, TileDefinition* >		s_tileDefinitions;

private:
	std::string		m_name				= "Default";
	AABB2			m_spriteUVBox		= AABB2();
	Rgba8			m_spriteTint		= Rgba8();
	Rgba8			m_texelColorForSet	= Rgba8();
	bool			m_allowSight		= true;
	bool			m_allowWalking		= true;
	bool			m_allowSwimming		= false;
	bool			m_allowFlying		= true;
};