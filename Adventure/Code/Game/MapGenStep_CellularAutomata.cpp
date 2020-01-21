#include "Game/MapGenStep_CellularAutomata.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/TileMetaData.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/IntVec2.hpp"


//---------------------------------------------------------------------------------------------------------
MapGenStep_CellularAutomata::MapGenStep_CellularAutomata( const XmlElement& element )
	: MapGenStep( element )
{
	m_neighborRadius		= ParseXmlAttribute( element, "NeighborRadius", m_neighborRadius );
	m_numNeighborsToChange	= ParseXmlAttribute( element, "NumNeighbors", m_numNeighborsToChange );

	std::string neighborDefName = ParseXmlAttribute( element, "IfNeighbor", "" );
	if( neighborDefName != "" )
	{
		m_ifNeighbor = TileDefinition::s_tileDefinitions[ neighborDefName ];
	}

	//---------------------------------------------------------------------------------------------------------
	std::string northTileDefName = ParseXmlAttribute( element, "IfNorthTile", "" );
	if( northTileDefName != "" )
	{
		m_ifNorthTile = TileDefinition::s_tileDefinitions[ northTileDefName ];
	}

	std::string southTileDefName = ParseXmlAttribute( element, "IfSouthTile", "" );
	if( southTileDefName != "" )
	{
		m_ifSouthTile = TileDefinition::s_tileDefinitions[ southTileDefName ];
	}

	std::string eastTileDefName = ParseXmlAttribute( element, "IfEastTile", "" );
	if( eastTileDefName != "" )
	{
		m_ifEastTile = TileDefinition::s_tileDefinitions[ eastTileDefName ];
	}

	std::string westTileDefName = ParseXmlAttribute( element, "IfWestTile", "" );
	if( westTileDefName != "" )
	{
		m_ifWestTile = TileDefinition::s_tileDefinitions[ westTileDefName ];
	}
}


//---------------------------------------------------------------------------------------------------------
void MapGenStep_CellularAutomata::RunStepOnce( Map& map )
{
	if( m_setTile == nullptr )
		return;

	IntVec2 mapDimensions = map.GetMapDefinition()->GetDimensions();
	int numTiles = mapDimensions.x * mapDimensions.y;

	float RandomChancePerTileInRange = m_chancePerTile.GetRandomInRange( *g_RNG );
	int numNeighborsNecessary = m_numNeighborsToChange.GetRandomInRange( *g_RNG );

	for( int tileIndex = 0; tileIndex < numTiles; ++tileIndex )
	{
		Tile& currentTile = map.GetTileAtIndex( tileIndex );
		if( !g_RNG->RollPercentChance( RandomChancePerTileInRange ) )
		{
			if( m_ifTile != nullptr && currentTile.GetTileDefinition() != m_ifTile )
				continue;
		}

		SetMetaDataIfNeighborsTile( map, currentTile, tileIndex, numNeighborsNecessary );
		map.ChangeTilesBasedOnMetaData();
	}
}


//---------------------------------------------------------------------------------------------------------
void MapGenStep_CellularAutomata::SetMetaDataIfNeighborsTile( Map& map, const Tile& currentTile, int currentTileIndex, int necessaryNumNeighbors )
{
	UNUSED( currentTile );
	int numberOfNeighboringTilesOfIfNeighbor = 0;
	IntVec2 mapDimensions = map.GetMapDefinition()->GetDimensions();
	//Doesnt account for going out of bounds yet
	numberOfNeighboringTilesOfIfNeighbor += CheckNeighborTileType( map, currentTileIndex, currentTileIndex + mapDimensions.x, DIRECTION_NORTH ); //North
	numberOfNeighboringTilesOfIfNeighbor += CheckNeighborTileType( map, currentTileIndex, currentTileIndex - mapDimensions.x, DIRECTION_SOUTH ); //South
	numberOfNeighboringTilesOfIfNeighbor += CheckNeighborTileType( map, currentTileIndex, currentTileIndex + 1, DIRECTION_EAST ); //East
	numberOfNeighboringTilesOfIfNeighbor += CheckNeighborTileType( map, currentTileIndex, currentTileIndex - 1, DIRECTION_WEST ); //West

	numberOfNeighboringTilesOfIfNeighbor += CheckNeighborTileType( map, currentTileIndex, currentTileIndex + 1 + mapDimensions.x ); //North-East
	numberOfNeighboringTilesOfIfNeighbor += CheckNeighborTileType( map, currentTileIndex, currentTileIndex + 1 - mapDimensions.x ); //South-East
	numberOfNeighboringTilesOfIfNeighbor += CheckNeighborTileType( map, currentTileIndex, currentTileIndex - 1 + mapDimensions.x ); //North-West
	numberOfNeighboringTilesOfIfNeighbor += CheckNeighborTileType( map, currentTileIndex, currentTileIndex - 1 - mapDimensions.x ); //South-West


	if( numberOfNeighboringTilesOfIfNeighbor < necessaryNumNeighbors )
	{
		TileMetaData& currentTileData = map.GetTileMetaDataAtIndex( currentTileIndex );
		currentTileData.m_tileTypeToChangeTo = nullptr;
	}
}

//---------------------------------------------------------------------------------------------------------
int MapGenStep_CellularAutomata::CheckNeighborTileType( Map& map, int currentTileIndex, int tileToCheckIndex, Direction tileToCheckDirection )
{
	IntVec2 mapDimensions = map.GetMapDefinition()->GetDimensions();
	int numTiles = mapDimensions.x * mapDimensions.y;

	if( tileToCheckIndex > numTiles - 1 || tileToCheckIndex < 0 )
		return 0;

	Tile& currentTile = map.GetTileAtIndex( currentTileIndex );
	Tile& tileToCheck = map.GetTileAtIndex( tileToCheckIndex );
	TileMetaData& currentTileData = map.GetTileMetaDataAtIndex( currentTileIndex );

	if( tileToCheck.GetTileDefinition() == m_ifNeighbor )
	{
		if( currentTile.GetTileDefinition() != m_ifNeighbor )
		{
			currentTileData.m_tileTypeToChangeTo = GetSetTileDef( tileToCheckDirection );
			return 1;
		}
	}
	return 0;
}


//---------------------------------------------------------------------------------------------------------
TileDefinition* MapGenStep_CellularAutomata::GetSetTileDef( Direction direction )
{
	if( direction == DIRECTION_NORTH && m_ifNorthTile != nullptr )
	{
		return m_ifNorthTile;
	}
	else if( direction == DIRECTION_SOUTH && m_ifSouthTile != nullptr )
	{
		return m_ifSouthTile;
	}
	else if( direction == DIRECTION_EAST && m_ifEastTile != nullptr )
	{
		return m_ifEastTile;
	}
	else if( direction == DIRECTION_WEST && m_ifWestTile != nullptr )
	{
		return m_ifWestTile;
	}

	return m_setTile;
}
