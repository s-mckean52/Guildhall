#pragma once
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Game/Tile.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include "Game/MapTileData.hpp"

struct AABB2;
class PlayerEntity;
class Game;
struct RaycastResult;
class Bullet;
class World;

enum CardinalDirection
{
	NORTH,
	EAST,
	SOUTH,
	WEST,

	NUM_CARDINAL_DIRECTIONS
};

class Map
{
public:
	~Map() {};
	Map( Game* theGame, World* theWorld, IntVec2 mapDimensions );

	void Render();
	void RenderEntityType( EntityType type ) const;
	void Update( float deltaSeconds );

	//---------------------------------------------------------------------------------------------------------
	// Accessors
	//
	const Vec2		GetStartPosition()	const { return m_startPosition; }
	const Vec2		GetExitPosition()	const { return m_exitPosition; }
	const IntVec2	GetMapDimensions()	const { return m_mapDimensions; }

	int				GetTileIndexForTileCoords	( const IntVec2& tileCoords )	const;
	IntVec2			GetTileCoordsForTileIndex	( int tileIndex )				const;
	Vec2			GetWorldPosForTileCoords	( const IntVec2& tileCoords )	const;
	IntVec2			GetTileCoordsForWorldPos	( const Vec2& worldPos )		const;
	int				GetTileIndexForWorldPos		( const Vec2& worldPos )		const;

	Entity*			GetEntity					( EntityType type, int index )	const;
	int				GetEntityCurrentTile		( Entity* entity )				const;
	TileType		GetEntityCurrentTileType	( Entity* entity )				const;

	//---------------------------------------------------------------------------------------------------------
	// Spawn Entity Methods
	//
	void			SpawnNumberOfEntites( int numEntitiesToSpawn );
	void			SpawnRandomEntity();
	void			SpawnEntityAtRandomPosition	( const EntityType& entityType );
	Entity*			SpawnEntity					( EntityType entityType, const Vec2& spawnPosition );
	void			SpawnExplosion					( const Vec2& position, float radius, float duration );
	void			AddEntityToMap				( Entity* entityToAdd );
	void			AddEntityToList				( Entity* entity, EntityList* entityList );

	//---------------------------------------------------------------------------------------------------------
	//  Map Creation Methods
	//
	void			CreateDefaultTiles( TileType defaultType );
	void			CreateBoundryTiles( TileType boundryType );
	void			SpawnWormsOfTileType( int numWorms, TileType wormType, int wormLength );
	void			RunWorm( const IntVec2& spawnTile, TileType wormType, int wormLength );
	IntVec2			MoveInRandomCardinalDirection( const IntVec2& tileCoords );
	bool			IsTileCoordWithinMapBounds( const IntVec2& tileCoords );
	void			SetExitTile( TileType goalType = TILE_TYPE_GOAL );
	void			SetPlayerStartTile( TileType startType = TILE_TYPE_GRASS );
	void			CreateSafeZones( TileType startZoneType = TILE_TYPE_GRASS, TileType exitZoneType = TILE_TYPE_GRASS );
	bool			HasPathToExit();
	bool			WasAdjacentTileSetAccessible( int tileIndex );
	bool			WasTileSetAccessible( int tileIndex );
	void			MakeInaccessibleTilesSolid( TileType tileType );

	//---------------------------------------------------------------------------------------------------------
	bool			HasLineOfSight( const Vec2& startPos, const Vec2& endPos );
	RaycastResult	Raycast( Vec2 startPoint, Vec2 fwdDir, float maxDistance);

	void			CheckExitMap();

private:
	//void CreateTiles();

	void DrawAllTiles();
	void DrawTile( std::vector<Vertex_PCU>& tileVerts, const Tile& tileToDraw );

	void UpdateEntities( float deltaSeconds );
	void UpdateEntityType( EntityType type, float deltaSeconds );

	void HandleCollisions();
	void HandleEntityCollisions( Entity* entity );

	void ResolveBulletCollisions( EntityType type );
	bool DidBulletCollideWithSolidTile( Bullet* bullet );
	void ResolveBulletOverlapsEntity( Bullet* bullet );

	void DeflectBulletOffEntity( Bullet* bulletToDeflect, Entity* entityToDeflectOffOf );
	bool AreOpposingFactions( Entity* A, Entity* B );
	bool DoEntitiesOverlap( Entity* A, Entity* B );
	void DrawLineBetweenEntities( Entity* A, Entity* B ) const;
	void DeleteGarbageEntities();
	void DeleteGarbageEntitiesOfType( EntityType type );

	void HandleTileCollision( Entity* entity, int currentTileIndex );

	bool IsPointInSolid( Vec2 pointToCheck );
	bool IsTileSolid( int tileIndex );

private:
	std::vector< Tile >			m_tiles;
	std::vector< Vertex_PCU >	m_mapTilesVerts;
	std::vector< MapTileData >	m_tileData;
	EntityList	m_entityLists[ NUM_ENTITY_TYPES ];


	Game* m_theGame = nullptr;
	World* m_theWorld = nullptr;

	IntVec2 m_mapDimensions;
	IntVec2 m_safeZoneSize;
	Vec2 m_exitPosition;
	Vec2 m_startPosition;
};