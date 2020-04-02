#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Plane2D::Plane2D( Vec2 const& toNormal, float toDistance )
{
	normal = toNormal;
	distance = toDistance;
}


//---------------------------------------------------------------------------------------------------------
Plane2D::Plane2D( Vec2 const& toNormal, Vec2 const& pointOnPlane )
{
	normal = toNormal;
	distance = DotProduct2D( toNormal, pointOnPlane );
}


//---------------------------------------------------------------------------------------------------------
Vec2 Plane2D::GetOrigin()
{
	return normal * distance;
}


//---------------------------------------------------------------------------------------------------------
float Plane2D::GetPointsDistanceFromPlane( Vec2 const& refPoint )
{
	return DotProduct2D( normal, refPoint ) - distance;
}


//---------------------------------------------------------------------------------------------------------
bool Plane2D::IsPointInFrontOfPlane( Vec2 const& refPoint )
{
	float pointDistanceFromPlane = GetPointsDistanceFromPlane( refPoint );
	if( pointDistanceFromPlane > 0.f )
	{
		return true;
	}
	return false;
}

