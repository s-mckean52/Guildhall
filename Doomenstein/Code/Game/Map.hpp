#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"

class Game;
class World;
class GPUMesh;
class Transform;
class Tile;
struct Vec2;

class Map
{
public:
	Map( Game* theGame, World* theWorld );
	~Map();

	void Update();
	void Render() const;

	bool IsTileSolid( Tile* tile ) const;
	IntVec2 GetTileXYCoordsForTileIndex( int tileIndex ) const;
	Vec3	GetTilePositionForTileIndex( int tileIndex ) const;

	void CreateMapVerts();
	void AppendVertsForTile( int tileIndex );
	void AppendVertsForOpenTile( int tileIndex );
	void AppendVertsForSolidTile( int tileIndex );


private:
	Game* m_game = nullptr;
	World* m_world = nullptr;

	IntVec2 m_dimensions = IntVec2( 10, 13 );

	GPUMesh* m_mapMesh = nullptr;
	std::vector<Tile*> m_tiles;
	std::vector<Vertex_PCUTBN> m_mapVerts;
};