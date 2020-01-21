#include "Game/MapDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapGenStep.hpp"
#include "Engine/Core/EngineCommon.hpp"

std::map< std::string, MapDefinition* > MapDefinition::s_mapDefinitions;

//---------------------------------------------------------------------------------------------------------
MapDefinition::MapDefinition( const XmlElement& element )
{
	m_name = ParseXmlAttribute( element, "name", m_name );

	int mapHeight	= ParseXmlAttribute( element, "height", m_dimensions.y );
	int mapWidth	= ParseXmlAttribute( element, "width", m_dimensions.x );
	m_dimensions	= IntVec2( mapWidth, mapHeight );

	std::string fillTileName = ParseXmlAttribute( element, "fillTile", "Grass" );
	m_defaultTile = TileDefinition::s_tileDefinitions[ fillTileName ];

	std::string edgeTileName = ParseXmlAttribute( element, "edgeTile", "Stone" );
	m_borderTile = TileDefinition::s_tileDefinitions[ edgeTileName ];
}


//---------------------------------------------------------------------------------------------------------
void MapDefinition::InitializeMapDefinitions()
{
	XmlDocument mapDefinitionXml = new XmlDocument();
	mapDefinitionXml.LoadFile( "Data/Definitions/MapDefinitions.xml" );
	GUARANTEE_OR_DIE( mapDefinitionXml.ErrorID() == 0, "MapDefinitions.xml does not exist in Run/Data/Definitions" );

	const XmlElement* rootElement = mapDefinitionXml.RootElement();
	const XmlElement* nextDefinition = rootElement->FirstChildElement();

	while( nextDefinition )
	{
		MapDefinition* newMapDefinition = new MapDefinition( *nextDefinition );
		s_mapDefinitions[ newMapDefinition->m_name ] = newMapDefinition;

		const XmlElement* mapGenStepElement = nextDefinition->FirstChildElement( "MapGenSteps" );
		mapGenStepElement = mapGenStepElement->FirstChildElement();
		while( mapGenStepElement )
		{
			newMapDefinition->AddMapGenStep( MapGenStep::CreateMapGenStep( *mapGenStepElement ) );
			mapGenStepElement = mapGenStepElement->NextSiblingElement();
		}
		
		nextDefinition = nextDefinition->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
void MapDefinition::AddMapGenStep( MapGenStep* newMapGenStep )
{
	m_mapGenSteps.push_back( newMapGenStep );
}

