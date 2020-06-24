#pragma once
#include <vector>

class Map;
class Game;

class World
{
public:
	World( Game* theGame );
	~World();

	void Render() const;
	void DebugRender() const;

	void Update( float deltaSeconds );

private:
	Game* m_theGame		= nullptr;
	Map* m_currentMap	= nullptr;

	std::vector<Map*> m_maps;
};