#pragma once
#include <vector>

class Map;
class Game;
class Enemy;
class Cursor;
class Player;

class World
{
public:
	World( Game* theGame );
	~World();

	void Render() const;
	void DebugRender() const;

	void Update( float deltaSeconds );

	void AddPlayerToCurrentMap( Player* player );
	Enemy* GetCursorOverlapEnemyOnCurrentMap( Cursor* cursor );

private:
	Game* m_theGame		= nullptr;
	Map* m_currentMap	= nullptr;

	std::vector<Map*> m_maps;
};