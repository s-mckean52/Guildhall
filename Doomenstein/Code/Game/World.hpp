#pragma once
#include "Engine/Core/StringUtils.hpp"
#include <vector>
#include <map>

class Game;
class Map;
class Entity;
struct Vec3;

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

	void CreateMapFromFilepath( char const* filepath );
	void SetCurrentMapByName( std::string mapName );
	void DeleteAllMaps();

	Entity* GetClostestEntityInForwardSector( Vec3 const& sectorStartPosition, float maxDistanceToCheck, Vec3 const& forwardDirNormalized, float aperatureDegrees );

private:
	Game*	m_game			= nullptr;
	Map*	m_currentMap	= nullptr;

	std::map<std::string, Map*> m_namedMaps;
};