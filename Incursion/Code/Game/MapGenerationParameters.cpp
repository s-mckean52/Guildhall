#include "Game/MapGenerationParameters.hpp"
#include "Game/Tile.hpp"
#include "Engine/Math/IntVec2.hpp"


//---------------------------------------------------------------------------------------------------------
MapGenerationParameters::MapGenerationParameters( const IntVec2& mapSize, TileType defaultTileType, TileType edgeTileType, TileType startTileType, TileType exitTileType, int numEntitiesToSpawn,
	// Worm Data Parameters
	TileType wormTileType1, int numWorms1, int wormLength1,
	TileType wormTileType2, int numWorms2, int wormLength2,
	TileType wormTileType3, int numWorms3, int wormLength3 )
	: size( mapSize )
	, defaultTile( defaultTileType )
	, edgeTile( edgeTileType )
	, startTile( startTileType )
	, exitTile( exitTileType )
	, numEntities( numEntitiesToSpawn )
{
	worms[0] = WormData( wormTileType1, numWorms1, wormLength1 );
	worms[1] = WormData( wormTileType2, numWorms2, wormLength2 );
	worms[2] = WormData( wormTileType3, numWorms3, wormLength3 );
}