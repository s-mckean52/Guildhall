#pragma once
#include "Game/Map.hpp"
#include "Game/MapGenerationParameters.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>

class PlayerEntity;
struct MapGenerationParameters;

class World
{
public:
	~World();
	World( Game* theGame );

	void Render() const;
	void Update( float deltaSeconds );
	Entity* GetCurrentMapEntity( EntityType type, int index ) const;
	IntVec2 GetCurrentMapDimensions() const;

	void MoveToNextMap();

	void GenerateNewMap( MapGenerationParameters mapParams );

private:
	Map*  m_currentMap = nullptr;
	std::vector< Map* > m_mapList;
	Game* m_theGame = nullptr;

	int m_currentMapNum = 0;

	//---------------------------------------------------------------------------------------------------------
	//		Map 1 Construction
	MapGenerationParameters m_map1 = MapGenerationParameters(
		IntVec2( MAP_1_SIZE_X, MAP_1_SIZE_Y ),
		MAP_1_DEFAULT_TILE_TYPE,
		MAP_1_EDGE_TILE_TYPE,
		MAP_1_START_TILE_TYPE,
		MAP_1_END_TILE_TYPE,
		MAP_1_NUM_ENTITIES_TO_SPAWN,

		//Worm Type			Worm Number			Worm Length
		MAP_1_WORM_1_TYPE,	MAP_1_WORM_1_NUM,	MAP_1_WORM_1_LENGTH
	);

	//		Map 2 Construction
	MapGenerationParameters m_map2 = MapGenerationParameters(
		IntVec2( MAP_2_SIZE_X, MAP_2_SIZE_Y ),
		MAP_2_DEFAULT_TILE_TYPE,
		MAP_2_EDGE_TILE_TYPE,
		MAP_2_START_TILE_TYPE,
		MAP_2_END_TILE_TYPE,
		MAP_2_NUM_ENTITIES_TO_SPAWN,

		//Worm Type			Worm Number			Worm Length
		MAP_2_WORM_1_TYPE,	MAP_2_WORM_1_NUM,	MAP_2_WORM_1_LENGTH,
		MAP_2_WORM_2_TYPE,	MAP_2_WORM_2_NUM,	MAP_2_WORM_2_LENGTH
	);

	//		Map 3 Construction
	MapGenerationParameters m_map3 = MapGenerationParameters(
		IntVec2( MAP_3_SIZE_X, MAP_3_SIZE_Y ),
		MAP_3_DEFAULT_TILE_TYPE,
		MAP_3_EDGE_TILE_TYPE,
		MAP_3_START_TILE_TYPE,
		MAP_3_END_TILE_TYPE,
		MAP_3_NUM_ENTITIES_TO_SPAWN,

		//Worm Type			Worm Number			Worm Length
		MAP_3_WORM_1_TYPE, MAP_3_WORM_1_NUM, MAP_3_WORM_1_LENGTH,
		MAP_3_WORM_2_TYPE, MAP_3_WORM_2_NUM, MAP_3_WORM_2_LENGTH,
		MAP_3_WORM_3_TYPE, MAP_3_WORM_3_NUM, MAP_3_WORM_3_LENGTH
	);
};