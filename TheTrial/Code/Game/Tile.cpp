#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/MeshUtils.hpp"

//---------------------------------------------------------------------------------------------------------
Tile::Tile( TileDefinition* tileDef, const IntVec2& tileCoords )
{
	m_tileDef = tileDef;
	m_tileCoords = tileCoords;
}


//---------------------------------------------------------------------------------------------------------
AABB2 Tile::GetWorldBounds() const
{
	Vec2 tileMinPos;
	tileMinPos.x = static_cast<float>( m_tileCoords.x );
	tileMinPos.y = static_cast<float>( m_tileCoords.y );

	Vec2 tileMaxPos = tileMinPos + TILE_DIMENSIONS;

	return AABB2( tileMinPos, tileMaxPos );
}


//---------------------------------------------------------------------------------------------------------
void Tile::SetTileDefinition( TileDefinition* tileDef )
{
	m_tileDef = tileDef;
}


//---------------------------------------------------------------------------------------------------------
void Tile::SetTileDefinitionWithTexel( const Rgba8& texelColor )
{
}


//---------------------------------------------------------------------------------------------------------
void Tile::AppendVertsForRender( std::vector<Vertex_PCU>& vertexArray ) const
{
	AABB2 worldBounds = GetWorldBounds();
	AABB2 tileDefUVBox = m_tileDef->GetSpriteUVBox();

	AppendVertsForAABB2D( vertexArray, worldBounds, m_tileDef->GetTint(), tileDefUVBox.mins, tileDefUVBox.maxes );
}
