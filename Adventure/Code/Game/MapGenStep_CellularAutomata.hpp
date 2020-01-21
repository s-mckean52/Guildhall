#pragma once
#include "Game/MapGenStep.hpp"
#include "Engine/Math/IntRange.hpp"

class	Map;
struct	Tile;

enum Direction
{
	DIRECTION_NONE = -1,
	DIRECTION_NORTH,
	DIRECTION_SOUTH,
	DIRECTION_EAST,
	DIRECTION_WEST
};

class MapGenStep_CellularAutomata : public MapGenStep
{
public:
	explicit MapGenStep_CellularAutomata( const XmlElement& element );

	virtual void	RunStepOnce( Map& map ) override;
	void			SetMetaDataIfNeighborsTile( Map& map, const Tile& currentTile, int currentTileIndex,  int necessaryNumNeighbors );
	int				CheckNeighborTileType( Map& map, int currentTileIndex, int tileToCheckIndex, Direction tileToCheckDirection = DIRECTION_NONE );
	TileDefinition* GetSetTileDef( Direction direction );

protected:
	TileDefinition* m_ifNeighbor			= nullptr;
	IntRange		m_numNeighborsToChange	= IntRange( 1 );
	int				m_neighborRadius		= 1;

	TileDefinition* m_ifNorthTile	= nullptr;
	TileDefinition* m_ifSouthTile	= nullptr;
	TileDefinition* m_ifEastTile	= nullptr;
	TileDefinition* m_ifWestTile	= nullptr;
};