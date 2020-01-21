#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Game/Tile.hpp"

struct RaycastResult
{
public:
	~RaycastResult() {};
	RaycastResult() {};
	explicit RaycastResult( Vec2 impactPos, bool didImpact, float impactDist, float impactFraction, TileType impactTileType );

	Vec2 m_impactPos;
	bool m_didImpact			= false;
	float m_impactDist			= 0.f;
	float m_impactFraction		= 0.f;
	TileType m_impactTileType	= INVALID_TILE_TYPE;
};
