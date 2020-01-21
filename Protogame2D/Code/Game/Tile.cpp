#include "Game/Tile.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
Rgba8 GetColorForTileType( Tile tile )
{
	switch( tile.m_tileType )
	{
	case TILE_TYPE_GRASS:
		return RGBA8_DARK_GREEN;

	case TILE_TYPE_STONE:
		return RGBA8_MEDIUM_DARK_GREY;

	case TILE_TYPE_GOAL:
		return RGBA8_MAGENTA;

	default:
		return RGBA8_WHITE;
	}

}


//---------------------------------------------------------------------------------------------------------
bool IsTileTypeSolid( Tile tile )
{
	switch( tile.m_tileType )
	{
	case TILE_TYPE_GRASS:
		return false;

	case TILE_TYPE_STONE:
		return true;

	case TILE_TYPE_GOAL:
		return false;

	default:
		return false;
	}
}


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
	//g_theRenderer->BindTexture( nullptr );
	//g_theRenderer->AppendVertsForAABB2D( tileVerts, GetTileBounds( tileToDraw.GetTileIndex() ), tileColor );
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