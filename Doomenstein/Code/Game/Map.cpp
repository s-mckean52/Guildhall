#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/World.hpp"
#include "Game/PlayerStart.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Camera.hpp"


//---------------------------------------------------------------------------------------------------------
Map::Map( Game* theGame, World* theWorld )
{
	m_game = theGame;
	m_world = theWorld;
}


//---------------------------------------------------------------------------------------------------------
Map::~Map()
{
	delete m_mapMesh;
	m_mapMesh = nullptr;

	delete m_playerStart;
	m_playerStart = nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Map::SpawnPlayer( Camera* playerCamera )
{
	m_playerStart->StartPlayer( playerCamera );
	g_theGame->PlaySpawnSound();
}