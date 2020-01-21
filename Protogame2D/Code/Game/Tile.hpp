#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"

struct Rgba8;
class Tile;

enum TileType
{
	INVALID_TILE_TYPE = -1,

	TILE_TYPE_GRASS,
	TILE_TYPE_STONE,
	TILE_TYPE_GOAL,

	NUM_TILE_TYPES
};

Rgba8	GetColorForTileType( Tile tile );
bool	IsTileTypeSolid( Tile tile );

class Tile
{
public:
	~Tile() {};
	Tile( IntVec2 tileCoords, TileType tileType );

	void Update( float deltaSeconds );
	void Render() const;
	void DebugRender() const;

	AABB2 GetBounds() const;

public:
	const IntVec2 m_tileCoords;
	TileType m_tileType = INVALID_TILE_TYPE;
};