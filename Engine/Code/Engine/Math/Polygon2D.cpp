#include "Engine/Math/Polygon2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
bool Polygon2D::IsValid() const
{
	if( ( GetVertexCount() > 2 ) && IsConvex() ) 
	{
		return true;
	}
	return false;
}


//---------------------------------------------------------------------------------------------------------
bool Polygon2D::IsConvex() const
{
	Vec2 currentEdgeStart;
	Vec2 currentEdgeEnd;
	Vec2 nextEdgeStart;
	Vec2 nextEdgeEnd;

	for( int edgeIndex = 0; edgeIndex < GetEdgeCount(); ++edgeIndex )
	{
		int nextEdgeIndex = ( edgeIndex + 1 ) % GetEdgeCount();
		GetEdge( edgeIndex, currentEdgeStart, currentEdgeEnd );
		GetEdge( nextEdgeIndex, nextEdgeStart, nextEdgeEnd );

		Vec2 currentEdge = currentEdgeEnd - currentEdgeStart;

		Vec2 edgeNormal = currentEdge.GetRotated90Degrees();
		edgeNormal.Normalize();
		Vec2 displacementCurrentEdgeStartToNextEdgeEnd = nextEdgeEnd - currentEdgeStart;

		if( DotProduct2D( edgeNormal, displacementCurrentEdgeStartToNextEdgeEnd ) < 0.f )
		{
			return false;
		}
	}
	return true;
}


//---------------------------------------------------------------------------------------------------------
bool Polygon2D::IsPointInside( Vec2 const& point ) const
{
	Vec2 edgeStart;
	Vec2 edgeEnd;

	for( int edgeIndex = 0; edgeIndex < GetEdgeCount(); ++edgeIndex )
	{
		GetEdge( edgeIndex, edgeStart, edgeEnd );

		Vec2 lineSegment = edgeEnd - edgeStart;
		Vec2 segmentNormal = lineSegment.GetRotated90Degrees();
		segmentNormal.Normalize();
		Vec2 displacementToPoint = point - edgeStart;
		if( DotProduct2D( segmentNormal, displacementToPoint ) < 0.f )
		{
			return false;
		}
	}
	return true;
}


//---------------------------------------------------------------------------------------------------------
float Polygon2D::GetDistance( Vec2 const& point ) const
{
	Vec2 closestPoint = GetClosestPoint( point );
	Vec2 displacement = point - closestPoint;
	return displacement.GetLength();
}


//---------------------------------------------------------------------------------------------------------
int Polygon2D::GetClosestEdgeIndex( Vec2 const& point ) const
{
	Vec2 edgeStart;
	Vec2 edgeEnd;
	int closestEdgeIndex = 0;
	float shortestDistanceSquared = 10000.f;

	for( int edgeIndex = 0; edgeIndex < GetEdgeCount(); ++edgeIndex )
	{
		GetEdge( edgeIndex, edgeStart, edgeEnd );
		Vec2 tempClosestPoint = GetNearestPointOnLineSegment2D( point, edgeStart, edgeEnd );
		Vec2 displacement = point - tempClosestPoint;
		float distanceSquaredToPoint = displacement.GetLengthSquared();
		if( shortestDistanceSquared > distanceSquaredToPoint )
		{
			shortestDistanceSquared = distanceSquaredToPoint;
			closestEdgeIndex = 0;
		}
	}

	return closestEdgeIndex;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Polygon2D::GetClosestPoint( Vec2 const& point ) const
{
	if( IsPointInside( point ) ) return point;

	return GetClosestPointOnEdge( point );
}


//---------------------------------------------------------------------------------------------------------
Vec2 Polygon2D::GetClosestPointOnEdge( Vec2 const& point ) const
{
	Vec2 edgeStart;
	Vec2 edgeEnd;
	Vec2 closestPoint;
	float shortestDistanceSquared = 10000.f;

	for( int edgeIndex = 0; edgeIndex < GetEdgeCount(); ++edgeIndex )
	{
		GetEdge( edgeIndex, edgeStart, edgeEnd );
		if( edgeIndex == 0 )
		{
			closestPoint = GetNearestPointOnLineSegment2D( point, edgeStart, edgeEnd );
			Vec2 displacement = point - closestPoint;
			shortestDistanceSquared = displacement.GetLengthSquared();
		}
		else
		{
			Vec2 tempClosestPoint = GetNearestPointOnLineSegment2D( point, edgeStart, edgeEnd );
			Vec2 displacement = point - tempClosestPoint;
			float distanceSquaredToPoint = displacement.GetLengthSquared();
			if( shortestDistanceSquared > distanceSquaredToPoint )
			{
				closestPoint = tempClosestPoint;
				shortestDistanceSquared = distanceSquaredToPoint;
			}
		}
	}

	return closestPoint;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Polygon2D::GetVertexAtIndex( int index ) const
{
	return m_points[ index ];
}


//---------------------------------------------------------------------------------------------------------
int Polygon2D::GetVertexCount() const
{
	return static_cast<int>( m_points.size() );
}


//---------------------------------------------------------------------------------------------------------
int Polygon2D::GetEdgeCount() const
{
	return GetVertexCount();
}


//---------------------------------------------------------------------------------------------------------
void Polygon2D::GetEdge( int edgeNumber, Vec2& out_start, Vec2& out_end ) const
{
	out_start = m_points[ edgeNumber ];
	if( edgeNumber < GetEdgeCount() - 1 )
	{
		out_end	= m_points[ static_cast<size_t>( edgeNumber ) + 1 ];
	}
	else
	{
		out_end = m_points[ 0 ];
	}
}


//---------------------------------------------------------------------------------------------------------
Polygon2D Polygon2D::GetTranslated( const Vec2& translation ) const
{
	std::vector<Vec2> newPoints;
	for( int pointIndex = 0; pointIndex < GetVertexCount(); ++pointIndex )
	{
		Vec2 newPointPosition = m_points[ pointIndex ] + translation;
		newPoints.push_back( newPointPosition );
	}
	return MakeFromLineLoop( &newPoints[ 0 ], static_cast<unsigned int>( newPoints.size() ) );
}


//---------------------------------------------------------------------------------------------------------
Polygon2D Polygon2D::GetRotatedRadians(float rotationRadians)
{
	std::vector<Vec2> rotatedPoints;
	for( int pointIndex = 0; pointIndex < GetVertexCount(); ++pointIndex )
	{
		Vec2 rotatedPoint = m_points[ pointIndex ].GetRotatedRadians( rotationRadians );
		rotatedPoints.push_back( rotatedPoint );
	}
	return MakeFromLineLoop( &rotatedPoints[ 0 ], static_cast<unsigned int>( rotatedPoints.size() ) );
}


//---------------------------------------------------------------------------------------------------------
STATIC Polygon2D Polygon2D::MakeFromLineLoop( Vec2 const* points, unsigned int pointCount )
{
	Polygon2D newPolygon;
	for( unsigned int pointIndex = 0; pointIndex < pointCount; ++pointIndex )
	{
		newPolygon.m_points.push_back( points[ pointIndex ] );
	}
	GUARANTEE_OR_DIE( newPolygon.IsValid(), "Bad points for convex poly" );
	return newPolygon;
}


//---------------------------------------------------------------------------------------------------------
STATIC Polygon2D Polygon2D::MakeFromPointCloud( Vec2 const* points, unsigned int pointCount )
{
	UNUSED( pointCount );
	UNUSED( points );
	Polygon2D newPolygon;
	GUARANTEE_OR_DIE( newPolygon.IsValid(), "Make polygon from cloud is not implemented" );
	return newPolygon;
}
