#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//---------------------------------------------------------------------------------------------------------
World::World( Game* theGame )
{
	m_theGame = theGame;
	//TODO: create maps from file;
	m_currentMap = new Map( theGame, this, "Data/Maps/TestMap.png" );
}


//---------------------------------------------------------------------------------------------------------
World::~World()
{
	delete m_currentMap;
	m_currentMap = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void World::Render() const
{
	m_currentMap->Render();


	if( g_isDebugDraw )
	{
		DebugRender();
	}
}


//---------------------------------------------------------------------------------------------------------
void World::DebugRender() const
{
}


//---------------------------------------------------------------------------------------------------------
void World::Update( float deltaSeconds )
{
	m_currentMap->Update( deltaSeconds );
}

