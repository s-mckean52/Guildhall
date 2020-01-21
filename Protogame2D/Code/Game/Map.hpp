#pragma once
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

struct AABB2;
class Entity;
class PlayerEntity;
class Game;

class Map
{
public:
	~Map() {};
	Map( Game* theGame, IntVec2 mapDimensions );

	void	Render();

	void	Update( float deltaSeconds );
	void	DrawTile( std::vector<Vertex_PCU>& tileVerts, const Tile& tileToDraw );
	int		GetTileIndexForTileCoords( const IntVec2& tileCoords ) const;
	IntVec2 GetTileCoordsForTileIndex( int tileIndex ) const;
	Vec2	GetWorldPosForTileCoords( const IntVec2& tileCoords ) const;
	IntVec2 GetTileCoordsForWorldPos( const Vec2& worldPos ) const;

	PlayerEntity* GetPlayer( int playerIndex ) const;

	int		GetEntityCurrentTile( Entity* entity );
	IntVec2 GetMapDimensions() { return m_mapDimensions; }

private:
	void CreateTiles();
	void UpdateEntities( float deltaSeconds );
	void HandleCollisions();
	void HandlePlayerCollisions( PlayerEntity* player);
	bool DoEntitiesOverlap( Entity* A, Entity* B );
	void DrawLineBetweenEntities( Entity* A, Entity* B ) const;
	void DeleteGarbageEntities();

	void HandleTileCollision( Entity* entity, int currentTileIndex );

private:
	std::vector< Tile >			m_tiles;
	std::vector< Entity* >		m_entities;
	std::vector< Vertex_PCU >	m_mapTilesVerts;
	std::vector< PlayerEntity* >m_players;

	Game* m_theGame = nullptr;

	IntVec2 m_mapDimensions;
	Vec2 m_tileSize;
	IntVec2 m_safeZoneSize;
};