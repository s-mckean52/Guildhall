#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
AABB2::AABB2( const Vec2& minPosition, const Vec2& maxPosition )
	: mins( minPosition )
	, maxes( maxPosition )
{
}


//---------------------------------------------------------------------------------------------------------
AABB2::AABB2( float minX, float minY, float maxX, float maxY )
{
	mins	= Vec2( minX, minY );
	maxes	= Vec2( maxX, maxY );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 AABB2::GetCenter() const
{
	return GetPointAtUV( Vec2( 0.5f, 0.5f ) );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 AABB2::GetDimensions() const
{
	return( maxes - mins );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 AABB2::GetNearestPoint( const Vec2& pointToCheck ) const
{
	Vec2 nearestPointOnAABB2;

	nearestPointOnAABB2.y = GetClamp( pointToCheck.y, mins.y, maxes.y );
	nearestPointOnAABB2.x = GetClamp( pointToCheck.x, mins.x, maxes.x );

	return nearestPointOnAABB2;
}


//---------------------------------------------------------------------------------------------------------
void AABB2::SetCenter( const Vec2& newCenterPoint )
{
	Vec2 displacementToNewCenter = newCenterPoint - GetCenter();
	Translate( displacementToNewCenter );
}


//---------------------------------------------------------------------------------------------------------
void AABB2::SetDimensions( const Vec2& newDimensions )
{
	Vec2 currentCenterPoint = GetCenter();

	mins = Vec2( 0.f, 0.f );
	maxes = newDimensions;

	SetCenter( currentCenterPoint );
}


//---------------------------------------------------------------------------------------------------------
void AABB2::AddDimensions( const Vec2& addedDimensions )
{
	Vec2 currentDimensions = GetDimensions();
	SetDimensions( currentDimensions + addedDimensions );
}


//---------------------------------------------------------------------------------------------------------
void AABB2::Translate( const Vec2& displacementVector )
{
	mins	+= displacementVector;
	maxes	+= displacementVector;
}


//---------------------------------------------------------------------------------------------------------
void AABB2::StretchToIncludePoint( const Vec2& pointToInclude )
{
	if( pointToInclude.x < mins.x )
	{
		mins.x += pointToInclude.x - mins.x;
	}
	else if( pointToInclude.x > maxes.x )
	{
		maxes.x += pointToInclude.x - maxes.x;
	}

	if( pointToInclude.y < mins.y )
	{
		mins.y += pointToInclude.y - mins.y;
	}
	else if( pointToInclude.y > maxes.y )
	{
		maxes.y += pointToInclude.y - maxes.y;
	}
}


//---------------------------------------------------------------------------------------------------------
const Vec2 AABB2::GetPointAtUV( const Vec2& uvs ) const
{
	float convertedX = RangeMapFloat( 0.f, 1.f, mins.x, maxes.x, uvs.x );
	float convertedY = RangeMapFloat( 0.f, 1.f, mins.y, maxes.y, uvs.y );

	return Vec2( convertedX, convertedY );
}


//---------------------------------------------------------------------------------------------------------
const Vec2 AABB2::GetUVForPoint( const Vec2& position ) const
{
	float convertedU = RangeMapFloat( mins.x, maxes.x, 0.f, 1.f, position.x );
	float convertedV = RangeMapFloat( mins.y, maxes.y, 0.f, 1.f, position.y );

	return Vec2( convertedU, convertedV );
}


//---------------------------------------------------------------------------------------------------------
AABB2 AABB2::GetBoxWithin( const Vec2& dimensions, const Vec2& alignment ) const
{
	Vec2 newBoxMins	= mins;
	newBoxMins += ( GetDimensions() - dimensions ) * alignment;
	Vec2 newBoxMaxes = newBoxMins + dimensions;

	return AABB2( newBoxMins, newBoxMaxes );
}


//---------------------------------------------------------------------------------------------------------
AABB2 AABB2::GetBoxAtLeft( float fractionOfWidth, float additionalWidth ) const
{
	Vec2 dimensions = GetDimensions();
	float newBoxWidth = dimensions.x * fractionOfWidth;
	float distanceToMoveMaxLeft = dimensions.x - newBoxWidth - additionalWidth ;

	return AABB2( mins.x, mins.y, maxes.x - distanceToMoveMaxLeft, maxes.y );
}


//---------------------------------------------------------------------------------------------------------
AABB2 AABB2::GetBoxAtRight( float fractionOfWidth, float additionalWidth ) const
{
	Vec2 dimensions = GetDimensions();
	float newBoxWidth = dimensions.x * fractionOfWidth;
	float distanceToMoveMinRight = dimensions.x - newBoxWidth - additionalWidth;

	return AABB2( mins.x + distanceToMoveMinRight, mins.y, maxes.x, maxes.y );
}


//---------------------------------------------------------------------------------------------------------
AABB2 AABB2::GetBoxAtBottom( float fractionOfHeight, float additionalWidth ) const
{
	Vec2 dimensions = GetDimensions();
	float newBoxHeight = dimensions.y * fractionOfHeight;
	float distanceToMoveMaxDown = dimensions.y - newBoxHeight - additionalWidth;

	return AABB2( mins.x, mins.y, maxes.x, maxes.y - distanceToMoveMaxDown );
}


//---------------------------------------------------------------------------------------------------------
AABB2 AABB2::GetBoxAtTop( float fractionOfHeight, float additionalWidth ) const
{
	Vec2 dimensions = GetDimensions();
	float newBoxHeight = dimensions.y * fractionOfHeight;
	float distanceToMoveMinUp = dimensions.y - newBoxHeight - additionalWidth;

	return AABB2( mins.x, mins.y + distanceToMoveMinUp, maxes.x, maxes.y );
}


//---------------------------------------------------------------------------------------------------------
float AABB2::GetOuterRadius() const
{
	Vec2 halfDimensions = GetDimensions() * 0.5f;
	return halfDimensions.x + halfDimensions.y;
}


//---------------------------------------------------------------------------------------------------------
float AABB2::GetInnerRadius() const
{
	Vec2 halfDimensions = GetDimensions() * 0.5f;

	if( halfDimensions.x > halfDimensions.y )
	{
		return halfDimensions.y;
	}
	return halfDimensions.x;
}


//---------------------------------------------------------------------------------------------------------
void AABB2::GetCornerPositions( Vec2* out_fourPoints ) const
{
	out_fourPoints[ 0 ] = mins;
	out_fourPoints[ 1 ] = Vec2( maxes.x, mins.y );
	out_fourPoints[ 2 ] = maxes;
	out_fourPoints[ 3 ] = Vec2( mins.x, maxes.y );
}


//---------------------------------------------------------------------------------------------------------
bool AABB2::IsPointInside( const Vec2& pointToCheck ) const
{
	if( pointToCheck.x > maxes.x || pointToCheck.x < mins.x )
	{
		return false;
	}
	else if( pointToCheck.y > maxes.y || pointToCheck.y < mins.y )
	{
		return false;
	}

	return true;
}


//---------------------------------------------------------------------------------------------------------
bool AABB2::SetFromText( const char* asText )
{
	Strings minsAndMaxesAsString = SplitStringOnDelimiter( asText, ',' );
	if( minsAndMaxesAsString.size() == 4 )
	{
		mins.x	= static_cast<float>( atof( minsAndMaxesAsString[ 0 ].c_str() ) );
		mins.y	= static_cast<float>( atof( minsAndMaxesAsString[ 1 ].c_str() ) );
		maxes.x = static_cast<float>( atof( minsAndMaxesAsString[ 2 ].c_str() ) );
		maxes.y = static_cast<float>( atof( minsAndMaxesAsString[ 3 ].c_str() ) );
		return true;
	}
	return false;
}

