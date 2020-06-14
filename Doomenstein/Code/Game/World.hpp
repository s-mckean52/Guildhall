#pragma once
#include "Engine/Core/StringUtils.hpp"
#include <map>

class Game;
class Map;


class World
{
public:
	World( Game* theGame );
	~World();

	void Update();
	void Render() const;

	Strings GetLoadedMapNames() const;
	void PrintLoadedMapsToDevConsole() const;

	void CreateMapFromFilepath( char const* filepath );
	void SetCurrentMapByName( std::string mapName );
	void DeleteAllMaps();


private:
	Game*	m_game			= nullptr;
	Map*	m_currentMap	= nullptr;

	std::map<std::string, Map*> m_namedMaps;
};