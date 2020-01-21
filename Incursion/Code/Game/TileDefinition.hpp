#pragma once
#include <vector>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"


class SpriteSheet;
struct Texture;
struct AABB2;


class TileDefinition
{
public:
	~TileDefinition() {};
	TileDefinition() {};
	explicit TileDefinition( Vec2 uvAtMins, Vec2 uvAtMaxes, Rgba8 tint, bool isSolid );

	bool		IsSolid() const				{ return m_isSolid; }
	Rgba8		GetTintColor() const		{ return m_tintColor; }
	Vec2		GetUVAtMins() const			{ return m_uvAtMins; }
	Vec2		GetUVAtMaxes() const		{ return m_uvAtMaxes; }
	AABB2		GetUVBounds() const;

	static void CreateTileDefinitions();

public:
	static Texture*					s_terrainTexture_8x8;
	static SpriteSheet*				s_terrainSpriteSheet;
	static std::vector<TileDefinition>	s_definitions;

private:
	const Vec2		m_uvAtMins;
	const Vec2		m_uvAtMaxes;
	const Rgba8		m_tintColor;
	const bool		m_isSolid = false;
};