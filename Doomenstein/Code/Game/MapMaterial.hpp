#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <string>
#include <map>


class SpriteSheet;
struct Texture;

class MapMaterial
{
private:
	MapMaterial( XmlElement const& xmlElement );

public:
	std::string		GetName() const				{ return m_name; }
	const Texture*	GetSpriteTexture() const	{ return m_spriteTexture; }
	AABB2			GetUVBox() const			{ return m_spriteUVBox; }

public:
	static void			CreateMapMaterialsFromXML( char const* filepath );
	static void			CreateMaterialSheet( XmlElement const& xmlElement );
	static MapMaterial* GetMaterialByName( std::string materialName );
	static MapMaterial* GetDefaultMaterial();

public:
	static std::string							s_defaultMaterialName;
	static std::map<std::string, MapMaterial*>	s_mapMaterials;
	static std::map<std::string, SpriteSheet*>	s_materialSheets;
private:
	std::string		m_name			= "";
	const Texture*	m_spriteTexture	= nullptr;
	AABB2			m_spriteUVBox	= AABB2();
};