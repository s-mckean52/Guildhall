#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"


//---------------------------------------------------------------------------------------------------------
SpriteSheet::SpriteSheet( const Texture& texture, const IntVec2& simpleGridLayout )
	: m_texture ( texture )
	, m_simpleGridSize( simpleGridLayout )
{
	float spriteSheetXStep = 1.f / simpleGridLayout.x;
	float spriteSheetYStep = 1.f / simpleGridLayout.y;

	int numSprites = simpleGridLayout.x * simpleGridLayout.y;
	
	for( int spriteIndex = 0; spriteIndex < numSprites; ++spriteIndex )
	{
		float currentMinU = spriteSheetXStep * static_cast<float>( spriteIndex % simpleGridLayout.x );
		float currenMinV = 1.f - ( spriteSheetYStep * static_cast<float>( 1 + ( spriteIndex / simpleGridLayout.y ) ) );
		Vec2 currentUVAtMins = Vec2( currentMinU, currenMinV );

		Vec2 currenUVAtMaxes = Vec2( currentUVAtMins.x + spriteSheetXStep, currentUVAtMins.y + spriteSheetYStep );

		m_spriteDefs.push_back( SpriteDefinition( *this, spriteIndex, currentUVAtMins, currenUVAtMaxes) );
	}
}


//---------------------------------------------------------------------------------------------------------
const SpriteDefinition& SpriteSheet::GetSpriteDefinition( int index ) const
{
	return m_spriteDefs[ index ];
}


//---------------------------------------------------------------------------------------------------------
void SpriteSheet::GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxes, int spriteIndex ) const
{
	SpriteDefinition currentSpriteDefinition = m_spriteDefs[ spriteIndex ];

	currentSpriteDefinition.GetUVs( out_uvAtMins, out_uvAtMaxes );
}


//---------------------------------------------------------------------------------------------------------
void SpriteSheet::GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxes, IntVec2 spritePosition ) const
{
	int spriteIndex = GetSpriteIndexFromPosition( spritePosition );
	GetSpriteUVs( out_uvAtMins, out_uvAtMaxes, spriteIndex );
}


//---------------------------------------------------------------------------------------------------------
int SpriteSheet::GetSpriteIndexFromPosition( IntVec2 spritePosition ) const
{
	return ( m_simpleGridSize.x * spritePosition.y ) + spritePosition.x;
}
