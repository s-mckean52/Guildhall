#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include <vector>

class SpriteDefinition;
struct Texture;

class SpriteSheet
{
public:
	~SpriteSheet() {};
	explicit SpriteSheet( const Texture& texture, const IntVec2& simpleGridLayout );

	const Texture&				GetTexture() const						{ return m_texture; }
	int							GetNumSprite() const					{ return static_cast<int>( m_spriteDefs.size() ); }
	const SpriteDefinition&		GetSpriteDefinition( int index ) const;
	void						GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxes, int spriteIndex ) const;
	void						GetSpriteUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxes, IntVec2 spritePosition ) const;
	int							GetSpriteIndexFromPosition( IntVec2 spritePosition ) const;

protected:
	IntVec2							m_simpleGridSize;
	const Texture&					m_texture;
	std::vector<SpriteDefinition>	m_spriteDefs;
};