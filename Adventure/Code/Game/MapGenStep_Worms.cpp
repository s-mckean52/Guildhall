#include "Game/MapGenStep_Worms.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/IntVec2.hpp"


//---------------------------------------------------------------------------------------------------------
MapGenStep_Worms::MapGenStep_Worms( const XmlElement& element )
	: MapGenStep( element )
{
	m_numWorms		= ParseXmlAttribute( element, "NumWorms", m_numWorms );
	m_wormLength	= ParseXmlAttribute( element, "WormLength", m_wormLength );
}


//---------------------------------------------------------------------------------------------------------
void MapGenStep_Worms::RunStepOnce( Map& map )
{
	if( m_setTile == nullptr )
		return;

	IntVec2 mapDimensions = map.GetMapDefinition()->GetDimensions();

	int randomNumWormsInRange = m_numWorms.GetRandomInRange( *g_RNG );
	for( int wormCount = 0; wormCount < randomNumWormsInRange; ++wormCount )
	{
		IntVec2 spawnTile;
		spawnTile.x = g_RNG->RollRandomIntInRange( 1, mapDimensions.x - 2 );
		spawnTile.y = g_RNG->RollRandomIntInRange( 1, mapDimensions.y - 2 );
		
		int randomWormLengthInRange = m_wormLength.GetRandomInRange( *g_RNG );
		for( int distanceTraveled = 0; distanceTraveled < randomWormLengthInRange; ++distanceTraveled )
		{
			int currentTileIndex = map.GetTileIndexForTileCoords( spawnTile );
			Tile& currentTile = map.GetTileAtIndex( currentTileIndex );

			if( m_ifTile != nullptr && currentTile.GetTileDefinition() != m_ifTile  )
			{
				spawnTile = map.MoveInRandomCardinalDirection( spawnTile );
				continue;
			}
			currentTile.SetTileDefinition( m_setTile );

			spawnTile = map.MoveInRandomCardinalDirection( spawnTile );
		}
	}
}
