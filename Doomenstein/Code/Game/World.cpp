#include "Game/World.hpp"
#include "Game/Map.hpp"
#include "Game/TileMap.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/FileUtils.hpp"


//---------------------------------------------------------------------------------------------------------
World::World( Game* theGame )
{
	m_game = theGame;
	std::string folderPath = "Data/Maps";
	g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR, Stringf( "Loading maps form %s...", folderPath.c_str() ) );

	Strings mapPaths = GetFileNamesInFolder( folderPath, "*.xml" );
	for( int mapPathIndex = 0; mapPathIndex < mapPaths.size(); ++mapPathIndex )
	{
		std::string mapFile = mapPaths[ mapPathIndex ];
		std::string mapPathToLoad = folderPath + "/" + mapFile;
		g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR, Stringf( "  Loading %s...", mapFile.c_str() ) );
		CreateMapFromFilepath( mapPathToLoad.c_str() );
	}

	std::string startMapName = g_gameConfigBlackboard.GetValue( "startMap", "MISSING" );
	if( startMapName == "MISSING" )
	{
		g_theConsole->ErrorString( "Starting map was not specified in GameConfig.xml" );
	}
	else
	{
		SetCurrentMapByName( startMapName );
	}
}


//---------------------------------------------------------------------------------------------------------
World::~World()
{
	DeleteAllMaps();
}


//---------------------------------------------------------------------------------------------------------
void World::Update()
{
	if( m_currentMap == nullptr )
		return;

	m_currentMap->Update();
}


//---------------------------------------------------------------------------------------------------------
void World::Render() const
{
	if( m_currentMap == nullptr )
		return;
	
	m_currentMap->Render();
}


//---------------------------------------------------------------------------------------------------------
Strings World::GetLoadedMapNames() const
{
	Strings loadedMapNames;

	for( auto mapIter = m_namedMaps.begin(); mapIter != m_namedMaps.end(); ++mapIter )
	{
		loadedMapNames.push_back( mapIter->first );
	}

	return loadedMapNames;
}


//---------------------------------------------------------------------------------------------------------
void World::PrintLoadedMapsToDevConsole() const
{
	Strings loadedMapNames = GetLoadedMapNames();
	
	g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR, "Loaded Maps:" );
	for( uint loadMapNameIndex = 0; loadMapNameIndex < loadedMapNames.size(); ++loadMapNameIndex )
	{
		std::string mapName = loadedMapNames[ loadMapNameIndex ];
		g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR, "  %s", mapName.c_str() );
	}
}


//---------------------------------------------------------------------------------------------------------
void World::CreateMapFromFilepath( char const* filepath )
{
	XmlDocument mapFile = new XmlDocument();
	mapFile.LoadFile( filepath );
	if( mapFile.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		g_theConsole->ErrorString( "Failed to load %s as XML",	filepath );
		g_theConsole->ErrorString( "  Error: %s",				mapFile.ErrorName() );
		g_theConsole->ErrorString( "  Error Line #%i",			mapFile.ErrorLineNum() );
		return;
	}

	XmlElement const* rootElement = mapFile.RootElement();
	std::string mapType = ParseXmlAttribute( *rootElement, "type", "MISSING" );

	Map* newMap = nullptr;
	if( mapType == "TileMap" )
	{
		newMap = new TileMap( m_game, this, *rootElement );
	}
	else
	{
		g_theConsole->ErrorString( "Tried to load invalid map type %s from %s", mapType.c_str(), filepath );
		return;
	}

	std::string mapName = GetFileNameWithoutExtension( filepath );
	m_namedMaps.insert( { mapName, newMap } );
	//g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR, Stringf( "Map %s Was Generated", mapName.c_str() ) );
}


//---------------------------------------------------------------------------------------------------------
void World::SetCurrentMapByName( std::string mapName )
{
	auto mapIterator = m_namedMaps.find( mapName );
	if( mapIterator != m_namedMaps.end() )
	{
		g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR, Stringf( "Setting current map to: %s...", mapIterator->first.c_str() ) );
		m_currentMap = mapIterator->second;
		m_currentMap->SpawnPlayer( m_game->GetPlayerCamera() );
	}
	else
	{
		g_theConsole->ErrorString( "The map %s does not exist", mapName.c_str() );
		PrintLoadedMapsToDevConsole();
	}
}


//---------------------------------------------------------------------------------------------------------
void World::DeleteAllMaps()
{
	auto mapIterator = m_namedMaps.begin();
	for( ;; )
	{
		if( mapIterator == m_namedMaps.end() )
			break;

		delete mapIterator->second;
		mapIterator->second = nullptr;
		++mapIterator;
	}
}