#pragma once
#include "Game/Map.hpp"
#include <vector>

class PlayerEntity;

class World
{
public:
	~World();
	World( Game* theGame, int numMaps, int mapMaxSizeX, int mapMaxSizeY );

	void Render() const;
	void Update( float deltaSeconds );
	PlayerEntity* GetCurrentMapPlayer( int playerIndex ) const;
	IntVec2 GetCurrentMapDimensions() const;

private:
	Map*  m_currentMap = nullptr;
	//std::vector< Map* > m_mapList; //Add this later for multiple maps?
	Game* m_theGame = nullptr;
};