#pragma once
#include "Game/Tile.hpp"

struct WormData
{
public:
	TileType tileType;
	int length = 0;
	int number = 0;

public:
	explicit WormData( TileType type = INVALID_TILE_TYPE, int numWorms = 0, int length = 0 );
};
