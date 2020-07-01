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
struct Tile;

class Map
{
public:
	Map( Game* theGame, World* theWorld, char const* imageFilepath );
	~Map();

	void Update( float deltaSeconds );
	void UpdateEntites( float deltaSeconds );
	void UpdateMapVerts( float deltaSeconds );

	void Render() const;
	void DebugRender() const;

	void CreateTilesFromImage( char const* filepath );
	void AddPlayerToMap( Player* player );
	void AddEntityToList( Entity* entity );
	Enemy* GetCursorOverlapEntity( Cursor* cursor );

private:
	Game*	m_theGame	= nullptr;
	World*	m_theWorld	= nullptr;

	Vec2 m_playerSpawnPosition;
	IntVec2 m_dimensions = IntVec2( 8, 8 );

	Player* m_player = nullptr;
	std::vector<Vertex_PCU> m_verts;
	std::vector<Tile*> m_tiles;
	std::vector<Entity*> m_entities;
};