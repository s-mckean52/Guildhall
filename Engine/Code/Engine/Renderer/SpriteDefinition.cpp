#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/Texture.hpp"

//---------------------------------------------------------------------------------------------------------
SpriteDefinition::SpriteDefinition( const SpriteSheet& spriteSheet, int spriteIndex, const Vec2& uvAtMins, const Vec2& uvAtMaxes )
	: m_spriteSheet( spriteSheet )
	, m_spriteIndex( spriteIndex )
	, m_uvMins( uvAtMins )
	, m_uvMaxes( uvAtMaxes )
{
}


//---------------------------------------------------------------------------------------------------------
void SpriteDefinition::GetUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxes ) const
{
	out_uvAtMins = m_uvMins;
	out_uvAtMaxes = m_uvMaxes;
}


//---------------------------------------------------------------------------------------------------------
const SpriteSheet& SpriteDefinition::GetSpriteSheet() const
{
	return m_spriteSheet;
}

//---------------------------------------------------------------------------------------------------------
const Texture& SpriteDefinition::GetTexture() const
{
	return m_spriteSheet.GetTexture();
}


//---------------------------------------------------------------------------------------------------------
float SpriteDefinition::GetAspect() const
{
	float spriteWidth = m_uvMaxes.x - m_uvMins.x;
	float spriteHeight = m_uvMaxes.y - m_uvMins.y;

	return ( spriteWidth / spriteHeight ) * m_spriteSheet.GetTexture().GetAspect();
}

