#pragma once
#include "Game/Map.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <vector>
#include <map>

class Game;
class Map;
class Entity;
struct Vec3;

struct WorldData
{
	char m_currentMapByName[50] = "";
	MapData m_currentMapData;
};

struct ConnectionData
{
	char m_currentMapByName[50] = "";
	//SpawnData m_entityData;
};

class World
{
public:
	World( Game* theGame );
	~World();

	void Update();
	void Render() const;

	Map* GetCurrentMap() const					{ return m_currentMap; }

	Strings GetLoadedMapNames() const;
	void PrintLoadedMapsToDevConsole() const;

	Map* GetLoadedMapByName( std::string const& mapName );
	void CreateMapFromFilepath( char const* filepath );
	void SetCurrentMap( Map* map );
	void SetCurrentMapByName( std::string mapName );
	void DeleteAllMaps();

	WorldData		GetWorldData();
	ConnectionData	GetConnectionData();
	void SpawnEntitiesFromSpawnData( SpawnData const& entitySpawnData );
	void UpdateEntitiesFromWorldData( WorldData const& worldData );

	Entity* GetClostestEntityInForwardSector( Vec3 const& sectorStartPosition, float maxDistanceToCheck, Vec3 const& forwardDirNormalized, float aperatureDegrees );

private:
	Game*	m_game			= nullptr;
	Map*	m_currentMap	= nullptr;

	std::map<std::string, Map*> m_namedMaps;
};