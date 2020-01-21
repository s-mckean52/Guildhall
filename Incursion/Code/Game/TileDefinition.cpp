#include "Game/TileDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Math/AABB2.hpp"

Texture*					TileDefinition::s_terrainTexture_8x8 = nullptr;
SpriteSheet*				TileDefinition::s_terrainSpriteSheet = nullptr;
std::vector<TileDefinition> TileDefinition::s_definitions;


//---------------------------------------------------------------------------------------------------------
TileDefinition::TileDefinition( Vec2 uvAtMins, Vec2 uvAtMaxes, Rgba8 tint, bool isSolid )
	: m_uvAtMins( uvAtMins )
	, m_uvAtMaxes( uvAtMaxes )
	, m_tintColor( tint )
	, m_isSolid( isSolid )
{
}


//---------------------------------------------------------------------------------------------------------
AABB2 TileDefinition::GetUVBounds() const
{
	AABB2 bounds;
	bounds.mins = m_uvAtMins;
	bounds.maxes = m_uvAtMaxes;
	return bounds;
}


//---------------------------------------------------------------------------------------------------------
void TileDefinition::CreateTileDefinitions()
{
	s_terrainTexture_8x8 = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/terrain_8x8.png" );
	s_terrainSpriteSheet = new SpriteSheet( *s_terrainTexture_8x8, IntVec2( 8, 8 ) );

	for( int tileTypeIndex = 0; tileTypeIndex < NUM_TILE_TYPES; ++tileTypeIndex )
	{
		Vec2 uvAtMins, uvAtMaxes;
		if( tileTypeIndex == TILE_TYPE_GRASS )
		{
			s_terrainSpriteSheet->GetSpriteUVs(uvAtMins, uvAtMaxes, 1 );
			s_definitions.push_back( TileDefinition( uvAtMins, uvAtMaxes, Rgba8::WHITE, false ) );
		}
		else if( tileTypeIndex == TILE_TYPE_STONE )
		{
			s_terrainSpriteSheet->GetSpriteUVs(uvAtMins, uvAtMaxes, 52 );
			s_definitions.push_back( TileDefinition( uvAtMins, uvAtMaxes, Rgba8::WHITE, true ) );
		}
		else if( tileTypeIndex == TILE_TYPE_MUD )
		{
			s_terrainSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxes, 21 );
			s_definitions.push_back( TileDefinition( uvAtMins, uvAtMaxes, Rgba8::WHITE, false ) );
		}		
		else if( tileTypeIndex == TILE_TYPE_WOOD )
		{
			s_terrainSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxes, 49 );
			s_definitions.push_back( TileDefinition( uvAtMins, uvAtMaxes, Rgba8::WHITE, false ) );
		}		
		else if( tileTypeIndex == TILE_TYPE_SHINGLES )
		{
			s_terrainSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxes, 44	 );
			s_definitions.push_back( TileDefinition( uvAtMins, uvAtMaxes, Rgba8::WHITE, true ) );
		}		
		else if( tileTypeIndex == TILE_TYPE_STONE_WALL )
		{
			s_terrainSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxes, 31 );
			s_definitions.push_back( TileDefinition( uvAtMins, uvAtMaxes, Rgba8::WHITE, true ) );
		}
		else if( tileTypeIndex == TILE_TYPE_SAND )
		{
			s_terrainSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxes, 22 );
			s_definitions.push_back( TileDefinition( uvAtMins, uvAtMaxes, Rgba8::WHITE, false ) );
		}
		else if( tileTypeIndex == TILE_TYPE_GOAL )
		{
			s_terrainSpriteSheet->GetSpriteUVs( uvAtMins, uvAtMaxes, 57 );
			s_definitions.push_back( TileDefinition( uvAtMins, uvAtMaxes, Rgba8::WHITE, false ) );
		}
	}
}