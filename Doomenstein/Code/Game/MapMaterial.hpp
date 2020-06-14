#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <string>
#include <map>


class SpriteSheet;

class MapMaterial
{
private:
	MapMaterial( XmlElement const& xmlElement );

public:
	std::string GetName() const			{ return m_name; }
	std::string GetSheetName() const	{ return m_sheetName; }
	AABB2		GetUVBox() const		{ return m_spriteUVBox; }

public:
	static void			CreateMapMaterialsFromXML( char const* filepath );
	static void			CreateMaterialSheet( XmlElement const& xmlElement );
	static MapMaterial* GetMaterialByName( std::string materialName );

public:
	static std::map<std::string, MapMaterial*> s_mapMaterials;
	static std::map<std::string, SpriteSheet*> s_materialSheets;

private:
	std::string m_name			= "";
	std::string m_sheetName		= "";
	AABB2		m_spriteUVBox	= AABB2();
};