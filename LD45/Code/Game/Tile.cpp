#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include <vector>

//---------------------------------------------------------------------------------------------------------
Tile::Tile( IntVec2 tileCoords, int index, TileType tileType )
	: m_tileCoords( tileCoords )
	, m_tileIndex( index )
	, m_tileType( tileType )
{
}