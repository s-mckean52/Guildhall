#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"

class MapRegion;

class Tile
{
public:
	explicit Tile( MapRegion* regionType, IntVec2 const& tileCoords );
	~Tile();

	bool		IsSolid() const;
	IntVec2		GetTileCoords() const	{ return m_tileCoords; }
	MapRegion*	GetRegionType() const	{ return m_regionType; }

private:
	IntVec2		m_tileCoords;
	MapRegion*	m_regionType = nullptr;
};