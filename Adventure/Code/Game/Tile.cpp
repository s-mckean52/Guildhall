#include "Game/Tile.hpp"
#include "Game/TileDefinition.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"


//---------------------------------------------------------------------------------------------------------
Tile::Tile( TileDefinition* tileDef, const IntVec2& tileCoords )
	: m_tileDef( tileDef )
	, m_tileCoords( tileCoords )
{
}


//---------------------------------------------------------------------------------------------------------
IntVec2 Tile::GetCoords() const
{
	return m_tileCoords;
}


//---------------------------------------------------------------------------------------------------------
TileDefinition* Tile::GetTileDefinition() const
{
	return m_tileDef;
}


//---------------------------------------------------------------------------------------------------------
void Tile::SetTileDefinition( TileDefinition* tileDef )
{
	m_tileDef = tileDef;
}


//---------------------------------------------------------------------------------------------------------
void Tile::SetTileDefinitionWithTexel( const Rgba8& texelColor )
{
	TileDefinition* tileDefToSetTo = TileDefinition::GetTileDefWithSetColor( texelColor );
	if( tileDefToSetTo != nullptr )
	{
		m_tileDef = tileDefToSetTo;
	}
}


//---------------------------------------------------------------------------------------------------------
AABB2 Tile::GetWorldBounds() const
{
	AABB2 tileBounds;
	tileBounds.mins.x = static_cast<float>(m_tileCoords.x);
	tileBounds.mins.y = static_cast<float>(m_tileCoords.y);
	tileBounds.maxes = tileBounds.mins + Vec2( 1.f, 1.f );

	return tileBounds;
}

//---------------------------------------------------------------------------------------------------------
void Tile::AppendVertsForRender( std::vector<Vertex_PCU>& vertexArray ) const
{
	AABB2 tileBoxBounds = GetWorldBounds();

	AppendVertsForAABB2D( vertexArray, tileBoxBounds, m_tileDef->GetTint(), m_tileDef->GetSpriteUVBox().mins, m_tileDef->GetSpriteUVBox().maxes );
}

