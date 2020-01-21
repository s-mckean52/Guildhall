#pragma once
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

struct AABB2;
class Entity;
class PlayerController;
class Game;

class Map
{
public:
	~Map() {};
	Map( Game* theGame, IntVec2 mapDimensions );

	void Render();

	void	Update( float deltaSeconds );
	void	DrawTile( std::vector<Vertex_PCU>& tileVerts, const Tile& tileToDraw );
	AABB2	GetTileBounds( int tileBounds ) const;
	int		GetTileIndexForTileCoords( const IntVec2& tileCoords ) const;
	IntVec2 GetTileCoordsForTileIndex( int tileIndex ) const;
	Vec2	GetWorldPosForTileCoords( const IntVec2& tileCoords ) const;
	IntVec2 GetTileCoordsForWorldPos( const Vec2& worldPos ) const;
	
	PlayerController* GetPlayer( int playerIndex ) const;

	int GetEntityCurrentTile( Entity* entity );
	IntVec2 GetMapDimensions() { return m_mapDimensions; }

private:
	void CreateTiles();
	void UpdateEntities( float deltaSeconds );
	void HandleCollisions();
	void HandlePlayerCollisions( PlayerController* player);
	bool DoEntitiesOverlap( Entity* A, Entity* B );
	void DrawLineBetweenEntities( Entity* A, Entity* B ) const;
	void DeleteGarbageEntities();

	void HandleTileCollision( Entity* entity, int currentTileIndex );
private:
	std::vector<Vertex_PCU> m_mapTilesVerts;
	std::vector< Entity > m_entities;
	std::vector< Tile > m_tiles;
	
	Game* m_theGame = nullptr;
	std::vector<PlayerController*> m_players;

	IntVec2 m_mapDimensions;
	Vec2 m_tileSize;
	IntVec2 m_safeZoneSize;
};