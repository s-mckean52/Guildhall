#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/DevConsole.hpp"


//---------------------------------------------------------------------------------------------------------
World::World( Game* theGame )
{
	m_game = theGame;

	CreateMapFromFilepath( "FileNameHere" );
	SetCurrentMapByName( "FileNameHere" );
}


//---------------------------------------------------------------------------------------------------------
World::~World()
{
	DeleteAllMaps();
}


//---------------------------------------------------------------------------------------------------------
void World::Update()
{
	m_currentMap->Update();
}


//---------------------------------------------------------------------------------------------------------
void World::Render() const
{
	m_currentMap->Render();
}


//---------------------------------------------------------------------------------------------------------
void World::CreateMapFromFilepath( char const* filepath )
{
	Map* newMap = new Map( m_game, this );
	m_namedMaps.insert( { filepath, newMap } );
	g_theConsole->PrintString( Rgba8::BLUE, Stringf( "Map %s Was Generated", filepath ) );
}


//---------------------------------------------------------------------------------------------------------
void World::SetCurrentMapByName( std::string mapName )
{
	auto mapIterator = m_namedMaps.find( mapName );
	if( mapIterator != m_namedMaps.end() )
	{
		m_currentMap = mapIterator->second;
		g_theConsole->PrintString( Rgba8::BLUE, Stringf( "Current Map is: %s", mapIterator->first.c_str() ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void World::DeleteAllMaps()
{
	auto mapIterator = m_namedMaps.begin();
	for (;; )
	{
		if( mapIterator == m_namedMaps.end() )
			break;

		delete mapIterator->second;
		mapIterator->second = nullptr;
		++mapIterator;
	}
}
