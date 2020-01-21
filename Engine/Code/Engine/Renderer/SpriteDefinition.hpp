#pragma once
#include "Engine/Math/Vec2.hpp"

class SpriteSheet;
struct Texture;

class SpriteDefinition
{
public:
	~SpriteDefinition() {};
	explicit SpriteDefinition( const SpriteSheet& spriteSheet, int spriteIndex, const Vec2& uvAtMins, const Vec2& uvAtMaxes );

	void				GetUVs( Vec2& out_uvAtMins, Vec2& out_uvAtMaxes ) const;
	const SpriteSheet&	GetSpriteSheet() const;
	const Texture&		GetTexture() const;
	float				GetAspect() const;

protected:
	const SpriteSheet&	m_spriteSheet;
	int					m_spriteIndex = -1;
	Vec2				m_uvMins;
	Vec2				m_uvMaxes = Vec2( 1.f, 1.f );
};