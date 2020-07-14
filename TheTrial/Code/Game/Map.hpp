#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class Game;
class World;
class Entity;
class Cursor;
class Enemy;
class Player;
class Actor;
struct Tile;
struct RaycastResult;

class Map
{
public:
	Map( Game* theGame, World* theWorld, char const* imageFilepath );
	~Map();

	void CleanUpEntities();
	void ClearEntities();

	void Update( float deltaSeconds );
	void UpdateEntites( float deltaSeconds );
	void UpdateMapVerts( float deltaSeconds );
	void ResolveEntityOverlaps();
	void PushActorsOutOfWalls();
	void PushActorOutOfTile( Actor* actorToPush, IntVec2 currentTileCoord, int xDir, int yDir );

	void Render() const;
	void DebugRender() const;

	void SetExitIsEnabled( bool isEnabled );
	bool IsPlayerInExit();

	Tile* GetTileByCoords( IntVec2 const& tileCoords );
	bool IsTileSolid( Tile* tile );

	void SpawnEnemy( int maxNumEnemies );
	void CreateTilesFromImage( char const* filepath );
	void AddPlayerToMap( Player* player );
	void AddEntityToList( Entity* entity );
	Enemy* GetDiscOverlapEnemy( Vec2 const& discCenterPosition, float discRadius );

public:
	RaycastResult Raycast( Vec2 const& startPosition, Vec2 const& direction, float maxDistance, Actor* ignoredActor = nullptr );
	RaycastResult RaycastAgainstTiles( Vec2 const& startPosition, Vec2 const& direction, float maxDistance );
	RaycastResult RaycastAgainstActors( Vec2 const& startPosition, Vec2 const& direction, float maxDistance, Actor* ignoredActor = nullptr );
	RaycastResult GetBestRaycast( std::vector<RaycastResult> results );

private:
	Game*	m_theGame	= nullptr;
	World*	m_theWorld	= nullptr;

	bool m_isExitEnabled = false;
	Vec2 m_playerSpawnPosition;
	Vec2 m_exitSpawnPosition;
	IntVec2 m_dimensions = IntVec2( 8, 8 );
	std::vector<Vec2> m_enemySpawnPositions;
	int m_enemyCount = 0;

	Player* m_player = nullptr;
	std::vector<Vertex_PCU> m_verts;
	std::vector<Tile*> m_tiles;
	std::vector<Entity*> m_entities;
	std::vector<Entity*> m_projectiles;
};