#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include <string>
#include <map>

class MapMaterial;

class MapRegion
{
private:
	explicit MapRegion( XmlElement const& xmlElement );

public:
	bool			IsSolid() const				{ return m_isSolid; }
	std::string		GetName() const				{ return m_name; }
	MapMaterial*	GetCeilingMaterial() const	{ return m_ceilingMaterial; }
	MapMaterial*	GetFloorMaterial() const	{ return m_floorMaterial; }
	MapMaterial*	GetSideMaterial() const		{ return m_sideMaterial; }

public:
	static void CreateMapRegionsFromXML( char const* filepath );
	static MapRegion* GetRegionByName( std::string regionTypeName );

public:
	static std::map<std::string, MapRegion*> s_mapRegions;

private:
	std::string		m_name				= "";
	bool			m_isSolid			= false;
	MapMaterial*	m_ceilingMaterial	= nullptr;
	MapMaterial*	m_floorMaterial		= nullptr;
	MapMaterial*	m_sideMaterial		= nullptr;
};