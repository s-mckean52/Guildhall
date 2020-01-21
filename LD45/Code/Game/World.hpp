#pragma once
#include "Game/Map.hpp"
#include <vector>

class PlayerController;

class World
{
private:
	Map*  m_currentMap = nullptr;
	std::vector<Map*> m_mapList;
	Game* m_theGame = nullptr;

public:
	~World() {};
	World( Game* theGame, int numMaps, int mapMaxSizeX, int mapMaxSizeY );

	void Render() const;
	void Update( float deltaSeconds );
	PlayerController* GetCurrentMapPlayer( int playerIndex ) const;
	IntVec2 GetCurrentMapDimensions() const;
};