#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include <string>

enum CardinalDirection
{
	NORTH,
	SOUTH,
	EAST,
	WEST,

	NUM_CARDINAL_DIRECTIONS
};

class	MapDefinition;
class	Actor;
struct	TileMetaData;

class Map
{
	friend class MapGenStep;

public:
	Map( std::string mapDefinitionName );

	void Update( float deltaSeconds );
	void Render() const;

	void CreateTiles();
	void RenderTiles() const;

	MapDefinition*	GetMapDefinition() const;
	IntVec2			GetTileCoordsForTileIndex( int tileIndex ) const;
	int				GetTileIndexForTileCoords( const IntVec2& tileCoords ) const;
	int				GetTileIndexForWorldPos( Vec2 position ) const;
	Tile&			GetTileAtIndex( int tileIndex );
	TileMetaData&	GetTileMetaDataAtIndex( int tileIndex );
	Vec2			GetPlayerPosition();

	IntVec2			MoveInRandomCardinalDirection( const IntVec2& tileCoords );
	void			ChangeTilesBasedOnMetaData();

private:
	void	SpawnActors();
	bool	IsTileCoordWithinMapBounds( const IntVec2& tileCoords );
	void	HandleCollisions();

	void	HandleTileCollision( Entity* entity, int currentTileIndex );
	int		GetEntityCurrentTile( Entity* entity );
	bool	EntityCanMoveOnTile( Entity* entity, TileDefinition* collidedTileDefinition );

private:
	Actor*						m_player		= nullptr;
	std::string					m_name			= "";
	MapDefinition*				m_mapDefinition = nullptr;
	IntVec2						m_dimensions;
	std::vector< Tile >			m_tiles;
	std::vector< TileMetaData >	m_tileMetaData;
};