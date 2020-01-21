#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerController.hpp"


//---------------------------------------------------------------------------------------------------------
World::World( Game* theGame, int numMaps, int mapMaxSizeX, int mapMaxSizeY )
	:m_theGame( theGame)
{
	for( int mapIndex = 0; mapIndex < numMaps; ++mapIndex )
	{
		m_mapList.push_back( new Map( m_theGame, IntVec2( mapMaxSizeX, mapMaxSizeY ) ) );
	}

	m_currentMap = m_mapList[ 0 ];
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
PlayerController* World::GetCurrentMapPlayer( int playerIndex ) const
{
	return m_currentMap->GetPlayer( playerIndex );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 World::GetCurrentMapDimensions() const
{
	return m_currentMap->GetMapDimensions();
}