#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/PlayerEntity.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/Game.hpp"
#include "Game/Explosion.hpp"
#include "Game/WormData.hpp"


//---------------------------------------------------------------------------------------------------------
World::~World()
{
	m_theGame = nullptr;

	for( int mapIndex = 0; mapIndex < m_mapList.size(); ++mapIndex )
	{
		delete m_mapList[ mapIndex ];
		m_mapList[ mapIndex ] = nullptr;
	}
}
//---------------------------------------------------------------------------------------------------------
World::World( Game* theGame )
	:m_theGame( theGame )
{
	TileDefinition::CreateTileDefinitions();
	Explosion::CreateExplosionSpriteSheet();

	GenerateNewMap( m_map1 );
	GenerateNewMap( m_map2 );
	GenerateNewMap( m_map3 );

	m_currentMap = m_mapList[ m_currentMapNum ];
	m_currentMap->AddEntityToMap( new PlayerEntity( m_theGame, m_currentMap, Vec2( 2.f, 2.f ), 0) );
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
Entity* World::GetCurrentMapEntity( EntityType type, int index ) const
{
	return m_currentMap->GetEntity( type, index );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 World::GetCurrentMapDimensions() const
{
	return m_currentMap->GetMapDimensions();
}


//---------------------------------------------------------------------------------------------------------
void World::MoveToNextMap()
{
	Entity* player = m_currentMap->GetEntity( ENTITY_TYPE_PLAYER, 0 );
	++m_currentMapNum;
	if( m_currentMapNum < m_mapList.size() )
	{
		m_currentMap = m_mapList[ m_currentMapNum ];
		m_currentMap->AddEntityToMap( player );
		player->SetPosition( m_currentMap->GetStartPosition() );
		player->SetMap( m_currentMap );
	}
	else
	{
		m_theGame->SetGameState( GAME_STATE_VICTORY );
	}
}

void World::GenerateNewMap( MapGenerationParameters mapParams )
{
	IntVec2 mapDimensions;
	mapDimensions.x = mapParams.size.x;
	mapDimensions.y = mapParams.size.y;

	Map* createdMap = new Map( m_theGame, this, mapDimensions );

	do
	{
		createdMap->CreateDefaultTiles( mapParams.defaultTile );
		createdMap->CreateBoundryTiles( mapParams.edgeTile );

		WormData* mapWormData = mapParams.worms; 
		createdMap->SpawnWormsOfTileType( mapWormData[0].number, mapWormData[0].tileType, mapWormData[0].length );
		createdMap->SpawnWormsOfTileType( mapWormData[1].number, mapWormData[1].tileType, mapWormData[1].length );
		createdMap->SpawnWormsOfTileType( mapWormData[2].number, mapWormData[2].tileType, mapWormData[2].length );

		createdMap->CreateSafeZones( mapParams.startTile, mapParams.exitTile );
		createdMap->SetExitTile();
		createdMap->SetPlayerStartTile( mapParams.startTile );

	} while( !createdMap->HasPathToExit() );

	createdMap->MakeInaccessibleTilesSolid( mapParams.edgeTile );
	createdMap->SpawnNumberOfEntites( mapParams.numEntities );

	m_mapList.push_back( createdMap );
}
