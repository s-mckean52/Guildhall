#include "Game/MapGenStep.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapGenStep_Mutate.hpp"
#include "Game/MapGenStep_Worms.hpp"
#include "Game/MapGenStep_CellularAutomata.hpp"
#include "Game/MapGenStep_FromImage.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"


//---------------------------------------------------------------------------------------------------------
MapGenStep::MapGenStep( const XmlElement& element )
{
	m_iterations	= ParseXmlAttribute( element, "iterations", m_iterations );
	m_chancePerTile	= ParseXmlAttribute( element, "chancePerTile", m_chancePerTile );
	
	std::string	ifTileName = ParseXmlAttribute( element, "ifTile", "" );
	if( ifTileName != "" )
	{
		m_ifTile = TileDefinition::s_tileDefinitions[ ifTileName ];
	}

	std::string setTileName = ParseXmlAttribute( element, "setTile", "" );
	if( setTileName != "" )
	{
		m_setTile = TileDefinition::s_tileDefinitions[ setTileName ];
	}
}


//---------------------------------------------------------------------------------------------------------
void MapGenStep::RunStep( Map& map )
{
	if( !g_RNG->RollPercentChance( m_chanceToRun ) )
	{
		return;
	}

	for( int iterations = 0; iterations < m_iterations; ++iterations )
	{
		RunStepOnce( map );
	}
}


//---------------------------------------------------------------------------------------------------------
MapGenStep* MapGenStep::CreateMapGenStep( const XmlElement& element )
{
	std::string mapGenStepType = element.Name();
	if( mapGenStepType == "Mutate" )				return new MapGenStep_Mutate( element );
	else if( mapGenStepType == "Worms" )			return new MapGenStep_Worms( element );
	else if( mapGenStepType == "CellularAutomata" )	return new MapGenStep_CellularAutomata( element );
	else if( mapGenStepType == "FromImage" )		return new MapGenStep_FromImage( element );

	ERROR_AND_DIE( "Not a valid Map Gen Step Name" );
}
