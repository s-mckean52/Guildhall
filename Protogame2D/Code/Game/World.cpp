#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerEntity.hpp"


//---------------------------------------------------------------------------------------------------------
World::~World()
{
	m_theGame = nullptr;

	delete m_currentMap;
	m_currentMap = nullptr;
}
//---------------------------------------------------------------------------------------------------------
World::World( Game* theGame, int numMaps, int mapMaxSizeX, int mapMaxSizeY )
	:m_theGame( theGame )
{
	UNUSED( numMaps );

	m_currentMap = new Map( m_theGame, IntVec2( mapMaxSizeX, mapMaxSizeY ) );
}

//---------------------------------------------------------------------------------------------------------
void World::Render() const
{
	m_currentMap->Render();
}


//---------------------------------------------------------------------------------------------------------
void World::Update( float deltaSeconds )
{
	m_currentMap->Update( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
PlayerEntity* World::GetCurrentMapPlayer( int playerIndex ) const
{
	return m_currentMap->GetPlayer( playerIndex );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 World::GetCurrentMapDimensions() const
{
	return m_currentMap->GetMapDimensions();
}