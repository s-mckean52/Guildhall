#pragma once
#include "Game/WormData.hpp"
#include "Game/Tile.hpp"

struct	IntVec2;

struct MapGenerationParameters
{
public:
	IntVec2		size;
	TileType	defaultTile;
	TileType	edgeTile;
	TileType	startTile;
	TileType	exitTile;
	int			numEntities;

	WormData worms[3] = {};

public:
	explicit MapGenerationParameters( const IntVec2& mapMaxSize, TileType defaultTile, TileType edgeTile, TileType startTileType, TileType exitTileType, int numEntitiesToSpawn,
												// Worm Data Parameters
												TileType wormTileType1 = INVALID_TILE_TYPE, int numWorms1 = 0, int wormLength1 = 0,
												TileType wormTileType2 = INVALID_TILE_TYPE, int numWorms2 = 0, int wormLength2 = 0,
												TileType wormTileType3 = INVALID_TILE_TYPE, int numWorms3 = 0, int wormLength3 = 0 );
};