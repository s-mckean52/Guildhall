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
	if( mapPaths.size() == 0 )
	{
		g_theConsole->ErrorString( "No maps found in the folder %s with extension 'xml'", folderPath.c_str() );
	}
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
		m_currentMap->SpawnPlayer( theGame->GetPlayerCamera() );
	}
}


//---------------------------------------------------------------------------------------------------------
World::~World()
{
	DeleteAllMaps();
}


//---------------------------------------------------------------------------------------------------------
void World::BeginFrame()
{
	if( m_currentMap == nullptr )
		return;

	m_currentMap->BeginFrame();
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
Map* World::GetLoadedMapByName( std::string const& mapName )
{
	auto mapIter = m_namedMaps.find( mapName );
	if( mapIter != m_namedMaps.end() )
	{
		return mapIter->second;
	}
	return nullptr;
}


//---------------------------------------------------------------------------------------------------------
void World::CreateMapFromFilepath( char const* filepath )
{
	const std::string mapFileRootNodeName = "MapDefinition";

	XmlDocument mapFile = new XmlDocument();
	mapFile.LoadFile( filepath );
	if( mapFile.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		g_theConsole->ErrorString( "Failed to load %s as XML",	filepath );
		g_theConsole->ErrorString( "  Error: %s",				mapFile.ErrorName());
		g_theConsole->ErrorString( "  Error Line #%i",			mapFile.ErrorLineNum() );
		return;
	}

	XmlElement const* rootElement = mapFile.RootElement();
	if( !IsStringEqual( rootElement->Name(), mapFileRootNodeName.c_str() ) )
	{
		g_theConsole->ErrorString( "Map file root node name was %s - should be %s", rootElement->Name(), mapFileRootNodeName.c_str() );
		return;
	}

	std::string mapType = ParseXmlAttribute( *rootElement, "type", "MISSING" );
	//TODO: Separate out into function
	Map* newMap = nullptr;
	std::string mapName = GetFileNameWithoutExtension( filepath );
	if( mapType == "TileMap" ) { newMap = new TileMap( m_game, this, mapName, *rootElement ); }
	else
	{
		g_theConsole->ErrorString(							"Tried to load unsupported map type %s", mapType.c_str(), filepath );
		g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR,	"Supported Map Types:" );
		g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR,	"  TileMap" );
		return;
	}

	m_namedMaps.insert( { mapName, newMap } );
}


//---------------------------------------------------------------------------------------------------------
void World::SetCurrentMap( Map* map )
{
	m_currentMap = map;
}


//---------------------------------------------------------------------------------------------------------
void World::SetCurrentMapByName( std::string mapName )
{
	auto mapIterator = m_namedMaps.find( mapName );
	if( mapIterator != m_namedMaps.end() )
	{
		g_theConsole->PrintString( DEV_CONSOLE_INFO_COLOR, Stringf( "Setting current map to: %s...", mapIterator->first.c_str() ) );
		SetCurrentMap( mapIterator->second );
		//m_currentMap->SpawnPlayer( m_game->GetPlayerCamera() );
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


//---------------------------------------------------------------------------------------------------------
WorldData World::GetWorldData()
{
	WorldData worldData;
	std::string currentMapName = m_currentMap->GetMapName();
	memcpy( &worldData.m_currentMapByName[0], &currentMapName[0], currentMapName.size() );
	worldData.m_currentMapData = m_currentMap->GetMapData();

	return worldData;
}


//---------------------------------------------------------------------------------------------------------
ConnectionData World::GetConnectionData()
{
	ConnectionData connectionData;
	std::string currentMapName = m_currentMap->GetMapName();
	memcpy( &connectionData.m_currentMapByName[0], &currentMapName[0], currentMapName.size() );
	memcpy( &connectionData.m_entityData, &m_currentMap->GetEntitySpawnData(), sizeof( SpawnData ) );
	return connectionData;
}


//---------------------------------------------------------------------------------------------------------
void World::SpawnEntitiesFromSpawnData( SpawnData const& spawnData )
{
	m_currentMap->DeleteAllEntities();
	for( int i = 0; i < 50; ++i )
	{
		if( spawnData.m_entitiesToSpawn[i].m_isUsed )
		{
			m_currentMap->SpawnEntityFromSpawnData( spawnData.m_entitiesToSpawn[i] );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void World::UpdateEntitiesFromWorldData( WorldData const& worldData )
{
	m_currentMap->UpdateEntitiesFromMapData( worldData.m_currentMapData );
}


//---------------------------------------------------------------------------------------------------------
Entity* World::GetClostestEntityInForwardSector( Vec3 const& sectorStartPosition, float maxDistanceToCheck, Vec3 const& forwardDirNormalized, float aperatureDegrees )
{
	return m_currentMap->GetClosestEntityInForwardSector( sectorStartPosition, maxDistanceToCheck, forwardDirNormalized, aperatureDegrees );
}
