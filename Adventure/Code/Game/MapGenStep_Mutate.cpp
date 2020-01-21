#include "Game/MapGenStep_Mutate.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/IntVec2.hpp"


//---------------------------------------------------------------------------------------------------------
MapGenStep_Mutate::MapGenStep_Mutate( const XmlElement& element )
	: MapGenStep( element )
{
	m_maxNumTilesToChange = ParseXmlAttribute( element, "NumTilesToChange", m_maxNumTilesToChange );
}


//---------------------------------------------------------------------------------------------------------
void MapGenStep_Mutate::RunStepOnce( Map& map )
{
	if( m_setTile == nullptr )
		return;

	IntVec2 mapDimensions = map.GetMapDefinition()->GetDimensions();
	int	mapSize = mapDimensions.x * mapDimensions.y;

	int numTilesChanged = 0;

	float RandomChancePerTileInRange = m_chancePerTile.GetRandomInRange( *g_RNG );
	for( int tileIndex = 0; tileIndex < mapSize; ++tileIndex )
	{
		if( numTilesChanged == m_maxNumTilesToChange )
			break;

		if( tileIndex < mapDimensions.x || tileIndex > mapSize - mapDimensions.x )
			continue;

		if( tileIndex % mapDimensions.x == 0 || tileIndex % mapDimensions.x == mapDimensions.x - 1 )
			continue;

		Tile& currentTile = map.GetTileAtIndex( tileIndex );
		if( currentTile.GetTileDefinition() == m_ifTile || m_ifTile == nullptr )
		{
			if( g_RNG->RollPercentChance( RandomChancePerTileInRange ) )
			{
				currentTile.SetTileDefinition( m_setTile );
				++numTilesChanged;
			}
		}
	}
}
