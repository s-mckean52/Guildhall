#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <map>
#include <string>

class TileDefinition;
class MapGenStep;

class MapDefinition
{
private:
	explicit MapDefinition( const XmlElement& element );

public:
	std::string		GetName()				{ return m_name; }
	IntVec2			GetDimensions()			{ return m_dimensions; }
	TileDefinition* GetDefaultTileDef()		{ return m_defaultTile; }
	TileDefinition* GetBorderTileDef()		{ return m_borderTile; }

	void			AddMapGenStep( MapGenStep* newMapGenStep );

public:
	static std::map< std::string, MapDefinition* > s_mapDefinitions;
	static void InitializeMapDefinitions();

public:
	std::vector< MapGenStep* >	m_mapGenSteps;

private:
	std::string		m_name			= "";
	IntVec2			m_dimensions	= IntVec2( 10, 10 );
	TileDefinition* m_defaultTile	= nullptr;
	TileDefinition* m_borderTile	= nullptr;
};