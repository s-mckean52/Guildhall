#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"


// Tile Methods
//---------------------------------------------------------------------------------------------------------
Tile::Tile( IntVec2 tileCoords, TileType tileType )
	: m_tileCoords( tileCoords )
	, m_tileType( tileType )
{
}

//---------------------------------------------------------------------------------------------------------
void Tile::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
void Tile::Render() const
{
}


//---------------------------------------------------------------------------------------------------------
void Tile::DebugRender() const
{

}


//---------------------------------------------------------------------------------------------------------
AABB2 Tile::GetBounds() const
{
	AABB2 bounds;
	bounds.mins.x = static_cast<float>( m_tileCoords.x );
	bounds.mins.y = static_cast<float>( m_tileCoords.y );
	bounds.maxes = Vec2( bounds.mins.x + 1.f, bounds.mins.y + 1.f );
	return bounds;
}