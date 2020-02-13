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
	return 0.f;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Polygon2D::GetClosestPoint( Vec2 const& point ) const
{
	if( IsPointInside( point ) ) return point;

	Vec2 edgeStart;
	Vec2 edgeEnd;
	Vec2 closestPoint;
	float shortestDistanceSquared;

	for( int edgeIndex = 0; edgeIndex < GetEdgeCount(); ++edgeIndex )
	{
		GetEdge( edgeIndex, edgeStart, edgeEnd );
		if( edgeIndex == 0 )
		{
			closestPoint = GetNearestPointOnLineSegment2D( point, edgeStart, edgeEnd );
			Vec2 displacement = closestPoint - edgeStart;
			shortestDistanceSquared = displacement.GetLengthSquared(); 
		}
		else
		{
			Vec2 tempClosestPoint = GetNearestPointOnLineSegment2D( point, edgeStart, edgeEnd );
			Vec2 displacement = tempClosestPoint - edgeStart;
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
	return MakeFromLineLoop( &newPoints[ 0 ], newPoints.size() );
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
