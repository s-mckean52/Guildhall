#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"

enum TileType
{
	INVALID_TILE_TYPE = -1,

	TILE_TYPE_GRASS,
	TILE_TYPE_STONE,
	TILE_TYPE_GOAL,

	NUM_TILE_TYPES
};

//GetColorForTileType
//IsTileTypeSolid

class Tile
{
public:
	~Tile() {};
	Tile( IntVec2 tileCoords, int tileIndex, TileType tileType );

	int GetTileIndex() const		{ return m_tileIndex; }
	IntVec2 GetTileCoords() const	{ return m_tileCoords; }
	TileType GetTileType() const	{ return m_tileType; }
	//Update
	//Render
	//DebugRender

	//AABB2 GetBounds()

	void SetTileType( TileType tileType ) { m_tileType = tileType; }

private: //public
	int m_tileIndex = -1; //Not needed
	TileType m_tileType = INVALID_TILE_TYPE;
	const IntVec2 m_tileCoords;
};