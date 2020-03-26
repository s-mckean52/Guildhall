#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
AABB3::AABB3( const Vec3& minPosition, const Vec3& maxPosition )
	: mins( minPosition )
	, maxes( maxPosition )
{
}


//---------------------------------------------------------------------------------------------------------
AABB3::AABB3( float minX, float minY, float minZ, float maxX, float maxY, float maxZ )
{
	mins	= Vec3( minX, minY, minZ );
	maxes	= Vec3( maxX, maxY, maxZ );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 AABB3::GetCenter() const
{
	return mins + ( maxes * 0.5f );
}


//---------------------------------------------------------------------------------------------------------
const Vec3 AABB3::GetDimensions() const
{
	return maxes - mins;
}


//---------------------------------------------------------------------------------------------------------
void AABB3::SetCenter( const Vec3& newCenterPoint )
{
	Vec3 displacementToNewCenter = newCenterPoint - GetCenter();
	Translate( displacementToNewCenter );
}


//---------------------------------------------------------------------------------------------------------
void AABB3::SetDimensions( const Vec3& newDimensions )
{
	Vec3 currentCenterPoint = GetCenter();

	mins = Vec2( 0.f, 0.f );
	maxes = newDimensions;

	SetCenter( currentCenterPoint );
}


//---------------------------------------------------------------------------------------------------------
void AABB3::Translate( const Vec3& displacementVector )
{
	mins	+= displacementVector;
	maxes	+= displacementVector;
}