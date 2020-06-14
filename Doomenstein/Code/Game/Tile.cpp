#include "Game/Tile.hpp"
#include "Game/MapRegion.hpp"

//---------------------------------------------------------------------------------------------------------
Tile::Tile( MapRegion* regionType, IntVec2 const& tileCoords )
{
	m_regionType = regionType;
	m_tileCoords = tileCoords;
}


//---------------------------------------------------------------------------------------------------------
Tile::~Tile()
{

}


//---------------------------------------------------------------------------------------------------------
bool Tile::IsSolid() const
{
	return m_regionType->IsSolid();
}
