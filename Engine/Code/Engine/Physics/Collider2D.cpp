#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Plane2D.hpp"


//---------------------------------------------------------------------------------------------------------
static bool DiscVDiscCollisionCheck( Collider2D const* col0, Collider2D const* col1 )
{
	DiscCollider2D const* disc0 = (DiscCollider2D*)col0;
	DiscCollider2D const* disc1 = (DiscCollider2D*)col1;

	return DoDiscsOverlap( disc0->m_worldPosition, disc0->m_radius, disc1->m_worldPosition, disc1->m_radius );
}


//---------------------------------------------------------------------------------------------------------
static bool DiscVPolygonCollisionCheck( Collider2D const* col0, Collider2D const* col1 )
{
	DiscCollider2D const* disc = (DiscCollider2D*)col0;
	PolygonCollider2D const* polygon = (PolygonCollider2D*)col1;

	return DoPolygonAndDiscOverlap( polygon->m_worldPolygon, disc->m_worldPosition, disc->m_radius );
}


//---------------------------------------------------------------------------------------------------------
static bool PolygonVPolygonCollisionCheck( Collider2D const* col0, Collider2D const* col1 )
{
	PolygonCollider2D const* polygon0 = (PolygonCollider2D*)col0;
	PolygonCollider2D const* polygon1 = (PolygonCollider2D*)col1;

	return DoPolygonsOverlap( polygon0->m_worldPolygon, polygon1->m_worldPolygon );
}


//---------------------------------------------------------------------------------------------------------
static collision_check_cb s_collisionCheck[NUM_COLLIDER_TYPE * NUM_COLLIDER_TYPE] ={
	/*				disc,							polygon		*/
	/*	   disc	*/	DiscVDiscCollisionCheck,		nullptr,
	/*	polygon	*/	DiscVPolygonCollisionCheck,		PolygonVPolygonCollisionCheck,
};


//---------------------------------------------------------------------------------------------------------
static bool DiscVDiscManifoldGeneration( Collider2D const* col0, Collider2D const* col1, Manifold2* manifold )
{
	DiscCollider2D const* disc0 = (DiscCollider2D*)col0;
	DiscCollider2D const* disc1 = (DiscCollider2D*)col1;

	Vec2 distance = disc0->m_worldPosition - disc1->m_worldPosition;
	float penetration = ( disc0->m_radius + disc1->m_radius ) - distance.GetLength();

	if( penetration > 0.f )
	{
		Vec2 normal = distance.GetNormalized();
		Vec2 contactPoint = disc0->m_worldPosition - ( normal * ( disc0->m_radius - ( penetration * 0.5f ) ) );
		manifold->SetContactEdge( contactPoint, contactPoint );
		manifold->collisionNormal = normal;
		manifold->penetrationDistance = penetration;
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
static bool DiscVPolygonManifoldGeneration( Collider2D const* col0, Collider2D const* col1, Manifold2* manifold )
{
	DiscCollider2D const* disc = (DiscCollider2D*)col0;
	PolygonCollider2D const* polygon = (PolygonCollider2D*)col1;

	float penetration = 0.f;

	Vec2 closestPointOnPolygonEdge = GetNearestPointOnEdgePolygon2D( disc->m_worldPosition, polygon->m_worldPolygon );
	Vec2 distance = disc->m_worldPosition - closestPointOnPolygonEdge;
	Vec2 normal = distance.GetNormalized();
	
	if( IsPointInsidePolygon2D( disc->m_worldPosition, polygon->m_worldPolygon ) )
	{
		normal = -normal;
		penetration = disc->m_radius + distance.GetLength();
	}
	else if( closestPointOnPolygonEdge == disc->m_worldPosition )
	{
		Vec2 edgeStart;
		Vec2 edgeEnd;
		Polygon2D worldPoly = polygon->m_worldPolygon;

		int overlappingEdgeIndex = worldPoly.GetClosestEdgeIndex( closestPointOnPolygonEdge );
		worldPoly.GetEdge( overlappingEdgeIndex, edgeStart, edgeEnd );

		Vec2 edgeNormal = edgeEnd - edgeStart;
		edgeNormal.Normalize();
		edgeNormal.RotateMinus90Degrees();

		normal = edgeNormal;
		penetration = disc->m_radius;
	}
	else
	{
		penetration = disc->m_radius - distance.GetLength();
	}


	if( penetration > 0.f )
	{
		manifold->SetContactEdge( closestPointOnPolygonEdge, closestPointOnPolygonEdge );
		manifold->collisionNormal = normal;
		manifold->penetrationDistance = penetration;
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
void GetClosestEdgeToOriginOnSimplex( std::vector<Vec2>& simplex, int& out_edgeStartIndex, Vec2& out_edgeStartPosition, Vec2& out_edgeEndPosition )
{
	float closestDistance = 100000000000.f;
	Vec2 closestEdgeStart;
	Vec2 closestEdgeEnd;
	for( int startPointIndex = 0; startPointIndex < simplex.size(); ++startPointIndex )
	{
		int endPointIndex = startPointIndex + 1;
		if( endPointIndex >= simplex.size() )
		{
			endPointIndex = 0;
		}
		Vec2 edgeStart = simplex[ startPointIndex ];
		Vec2 edgeEnd = simplex[ endPointIndex ];

		Vec2 edge = edgeEnd - edgeStart;
		Vec2 edgeNormal = edge.GetNormalized();
		edgeNormal.RotateMinus90Degrees();

		float distanceFromOrigin = DotProduct2D( edgeStart, edgeNormal );

		if( distanceFromOrigin < closestDistance )
		{
			out_edgeStartIndex = startPointIndex;
			closestDistance = distanceFromOrigin;
			closestEdgeStart = edgeStart;
			closestEdgeEnd = edgeEnd;
		}
	}
	out_edgeStartPosition = closestEdgeStart;
	out_edgeEndPosition = closestEdgeEnd;
}


//---------------------------------------------------------------------------------------------------------
bool ClipSegmentToSegement( Vec2 const& toClipStart, Vec2 const& toClipEnd, Vec2 const& refEdgeStart, Vec2 const& refEdgeEnd, Vec2& out_clippedStart, Vec2& out_clippedEnd )
{
	Vec2 refEdgeNormalized = ( refEdgeEnd - refEdgeStart ).GetNormalized();
	float refStart = DotProduct2D( refEdgeNormalized, refEdgeStart );
	float refEnd = DotProduct2D( refEdgeNormalized, refEdgeEnd );
	float toClipSegmentStart = DotProduct2D( refEdgeNormalized, toClipStart );
	float toClipSegmentEnd = DotProduct2D( refEdgeNormalized, toClipEnd );
	
	float clippedSegmentStart = Maxf( refStart, Minf( toClipSegmentStart, toClipSegmentEnd ) );
	float clippedSegmentEnd = Minf( refEnd, Maxf( toClipSegmentStart, toClipSegmentEnd ) );

	if( clippedSegmentEnd < clippedSegmentStart )
	{
		return false;
	}

	Vec2 clippedSegementStartPosition = RangeMapFloatToVec2( toClipSegmentStart, toClipSegmentEnd, toClipStart, toClipEnd, clippedSegmentStart );
	Vec2 clippedSegementEndPosition = RangeMapFloatToVec2( toClipSegmentStart, toClipSegmentEnd, toClipStart, toClipEnd, clippedSegmentEnd );

	out_clippedStart = clippedSegementStartPosition;
	out_clippedEnd = clippedSegementEndPosition;
	return true;
}


//---------------------------------------------------------------------------------------------------------
static bool PolygonVPolygonManifoldGeneration( Collider2D const* col0, Collider2D const* col1, Manifold2* manifold )
{
	const float debugduration = 10.f;

	PolygonCollider2D const* polygon0 = (PolygonCollider2D*)col0;
	PolygonCollider2D const* polygon1 = (PolygonCollider2D*)col1;

	Polygon2D polygonA = polygon0->m_worldPolygon; //me
	Polygon2D polygonB = polygon1->m_worldPolygon; //them

	float penetration = 0.f;
	Vec2 collisionNormal;

	std::vector<Vec2> simplex;
	if( !DoPolygonsOverlap( polygonA, polygonB, &simplex ) ) return false;

	//Set Winding order to counter clockwise
	Vec2 firstEdgeNormal = simplex[ 1 ] - simplex[ 0 ];
	firstEdgeNormal.Rotate90Degrees();
	if( DotProduct2D( firstEdgeNormal, simplex[ 2 ] ) < 0.f )
	{
		Vec2 tempPoint = simplex[ 1 ];
		simplex[ 1 ] = simplex[ 2 ];
		simplex[ 2 ] = tempPoint;
	}
	//Polygon2D simplexPoly = Polygon2D::MakeFromLineLoop( &simplex[0], static_cast<unsigned int>( simplex.size() ) );

	for( ;; )
	{
		/*if( !ExpandMinkowskiDifference( polygonA, polygonB, simplexPoly )
		{
			normalPoint = simplexPoly.GetClosestPointOnEdgeNearestToPoint();
			penetration = normalPoint.GetLength();
			collisionNormal = -normalPoint.GetNormalized;
			if( normalPoint == Vec2::ZERO )
				collisionNormal = edgeNormal;
		}

		manifold->collisionNormal = collisionNormal;
		manifold->penetrationDistance = penetration;

		Vec2 pointOnCullingPlane = polygonB.GetSupportPointInDirection( collisionNormal );
		Plane2D cullingPlane = Plane2D( collisionNormal, pointOnCullingPlane );
		Vec2 planeTangent = cullingPlane.GetTangent();
		Segment2 refSegment = polygonB.GetMinAndMaxPointsOnPlane();
		if( refSegment.IsPoint() )
		{
			manifold->collisionSegment = refSegment;
			return true;
		}

		std::vector<Vec2> contacts = polygonA.GetContactsOnEdgeBehindPlane( refSegment, cullingPlane );
		Segment2D collisionSegment = GetMinAndMaxPointsInDirection( contacts, cullingPlane );
		manifold->collisionSegment = collisionSegment;*/

		int edgeStartIndex = 0;
		Vec2 edgeStartPosition;
		Vec2 edgeEndPosition;
		GetClosestEdgeToOriginOnSimplex( simplex, edgeStartIndex, edgeStartPosition, edgeEndPosition );


		Vec2 closestEdge = edgeEndPosition - edgeStartPosition;
		Vec2 closestEdgeNormal = closestEdge.GetNormalized();
		closestEdgeNormal.RotateMinus90Degrees();
		float closestDistance = DotProduct2D( edgeStartPosition, closestEdgeNormal );

		Vec2 simplexSupport = polygonA.GetSupportPointInDirection( closestEdgeNormal ) - polygonB.GetSupportPointInDirection( -closestEdgeNormal );
		float supportDistanceFromOrigin = DotProduct2D( closestEdgeNormal, simplexSupport );
		if( ApproximatelyEqual( supportDistanceFromOrigin, closestDistance ) )
		{
			Vec2 normalPoint = GetNearestPointOnLineSegment2D( Vec2::ZERO, edgeStartPosition, edgeEndPosition );
			collisionNormal = -normalPoint.GetNormalized();
			if( normalPoint == Vec2::ZERO )
			{
				collisionNormal = closestEdgeNormal;
			}
			penetration = normalPoint.GetLength();
			break;
		}
		else
		{
			std::vector<Vec2> tempSimplex;
			for( int simplexIndex = 0; simplexIndex < simplex.size(); ++simplexIndex )
			{
				tempSimplex.push_back( simplex[ simplexIndex ] );
				if( simplexIndex == edgeStartIndex )
				{
					tempSimplex.push_back( simplexSupport );
				}
			}
			simplex = tempSimplex;
		}
	}

	manifold->collisionNormal = collisionNormal;
	manifold->penetrationDistance = penetration;


	//GetCullingSegment
	Vec2 pointOnCullingPlane = polygonB.GetSupportPointInDirection( collisionNormal );
	Plane2D cullingPlane = Plane2D( collisionNormal, pointOnCullingPlane );
	Vec2 planeTangent = cullingPlane.normal.GetRotatedMinus90Degrees();
	float minPointDistance = 100000000.f;
	float maxPointDistance = -100000000.f;
	for( int polgonBVertIndex = 0; polgonBVertIndex < polygonB.GetVertexCount(); ++polgonBVertIndex )
	{
		Vec2 vert = polygonB.GetVertexAtIndex( polgonBVertIndex );
		float vertexDistanceFromPlane = cullingPlane.GetPointsDistanceFromPlane( vert );
		if( abs( vertexDistanceFromPlane ) < 0.0001f )
		{
			float vertexDistanceAlongPlane = DotProduct2D( planeTangent, vert );
			if( minPointDistance > vertexDistanceAlongPlane )
			{
				minPointDistance = vertexDistanceAlongPlane;
			}

			if( maxPointDistance < vertexDistanceAlongPlane )
			{
				maxPointDistance = vertexDistanceAlongPlane;
			}
		}
	}

	Vec2 planeOrigin = cullingPlane.GetOrigin();
	Vec2 minCullingPoint = planeOrigin + ( planeTangent * minPointDistance );
	Vec2 maxCullingPoint = planeOrigin + ( planeTangent * maxPointDistance );

	if( minCullingPoint == maxCullingPoint )
	{
		manifold->SetContactEdge( minCullingPoint, maxCullingPoint );
		return true;
	}

	Vec2 cullingSegmentNormal = ( maxCullingPoint - minCullingPoint ).GetNormalized();
	cullingSegmentNormal.RotateMinus90Degrees();

	std::vector<Vec2> contacts;
	for( int polygonAEdgeIndex = 0; polygonAEdgeIndex < polygonA.GetEdgeCount(); ++polygonAEdgeIndex )
	{
		Vec2 segmentStart;
		Vec2 segmentEnd;
		polygonA.GetEdge( polygonAEdgeIndex, segmentStart, segmentEnd );

		Vec2 segmentNormal = ( segmentEnd - segmentStart ).GetNormalized();
		segmentNormal.RotateMinus90Degrees();

		if( DotProduct2D( segmentNormal, cullingSegmentNormal ) > 0.f )
		{
			Vec2 clippedSegmentStart;
			Vec2 clippedSegmentEnd;
			if( ClipSegmentToSegement( segmentStart, segmentEnd, minCullingPoint, maxCullingPoint, clippedSegmentStart, clippedSegmentEnd ) ) 
			{
				if( !cullingPlane.IsPointInFrontOfPlane( clippedSegmentStart ) )
				{
					contacts.push_back( clippedSegmentStart );
				}
				if( !cullingPlane.IsPointInFrontOfPlane( clippedSegmentEnd ) )
				{
					contacts.push_back( clippedSegmentEnd );
				}
			}

		}
	}

	Vec2 minContact;
	Vec2 maxContact;
	float maxContactDistance = -1000000000.f;
	float minContactDistance = 1000000000.f;
	Vec2 cullingTangent = cullingSegmentNormal.GetRotatedMinus90Degrees();
	for( int contactIndex = 0; contactIndex < contacts.size(); ++ contactIndex )
	{
		Vec2 contact = contacts[ contactIndex ];
		float distanceAlongCollisionTangent =  DotProduct2D( contact, cullingTangent );

		if( distanceAlongCollisionTangent < minContactDistance )
		{
			minContact = contact;
			minContactDistance = distanceAlongCollisionTangent;
		}

		if (distanceAlongCollisionTangent > maxContactDistance)
		{
			maxContact = contact;
			maxContactDistance = distanceAlongCollisionTangent;
		}
	}

	manifold->SetContactEdge( minContact, maxContact );
	return true;
}


//---------------------------------------------------------------------------------------------------------
static manifold_check_cb s_manifoldCheck[NUM_COLLIDER_TYPE * NUM_COLLIDER_TYPE] ={
	/*				disc,								polygon		*/
	/*	   disc	*/	DiscVDiscManifoldGeneration,		nullptr,
	/*	polygon	*/	DiscVPolygonManifoldGeneration,		PolygonVPolygonManifoldGeneration,
};


//---------------------------------------------------------------------------------------------------------
void Collider2D::Move( Vec2 const& movement )
{
	m_rigidbody->m_worldPosition += movement;
	UpdateWorldShape();
}


//---------------------------------------------------------------------------------------------------------
float Collider2D::GetMass() const
{
	if( m_rigidbody != nullptr )
	{
		return m_rigidbody->m_mass;
	}
	return 1.f;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Collider2D::GetVelocity() const
{
	if( m_rigidbody != nullptr )
	{
		return m_rigidbody->GetVelocity();
	}
	return Vec2();
}


//---------------------------------------------------------------------------------------------------------
void Collider2D::MarkForDestroy( bool isMarkedForDestroy )
{
	m_isMarkedForDestroy = isMarkedForDestroy;
}


//---------------------------------------------------------------------------------------------------------
bool Collider2D::Intersects( Collider2D const* other ) const
{
	Collider2DType myType = GetType();
	Collider2DType otherType = other->GetType();

	if( !WorldBoundsIntersect( other ) ) return false;

	if( myType <= otherType )
	{
		int collisionCheckIndex = otherType * NUM_COLLIDER_TYPE + myType;
		collision_check_cb collisionCallback = s_collisionCheck[collisionCheckIndex];
		return collisionCallback( this, other );
	}
	else {
		// flip the types when looking into the index.
		int collisionCheckIndex = myType * NUM_COLLIDER_TYPE + otherType;
		collision_check_cb collisionCallback = s_collisionCheck[collisionCheckIndex];
		return collisionCallback( other, this );
	}
}


//---------------------------------------------------------------------------------------------------------
bool Collider2D::GetManifold( Collider2D const* other, Manifold2* manifold )
{
	Collider2DType myType = GetType();
	Collider2DType otherType = other->GetType();

	if( !WorldBoundsIntersect( other ) ) return false;

	if( myType <= otherType )
	{
		int manifoldCheckIndex = otherType * NUM_COLLIDER_TYPE + myType;
		manifold_check_cb manifoldCallback = s_manifoldCheck[ manifoldCheckIndex ];
		return manifoldCallback( this, other, manifold );
	}
	else {
		// flip the types when looking into the index.
		int manifoldCheckIndex = myType * NUM_COLLIDER_TYPE + otherType;
		manifold_check_cb manifoldCallback = s_manifoldCheck[ manifoldCheckIndex ];
		return manifoldCallback( other, this,  manifold );
	}
}


//---------------------------------------------------------------------------------------------------------
bool Collider2D::WorldBoundsIntersect( Collider2D const* other ) const
{
	return DoAABB2sOverlap( GetWorldBounds(), other->GetWorldBounds() );
	//return DoDiscsOverlap( m_worldBoundsCenter, m_worldBoundsRadius, other->m_worldBoundsCenter, other->m_worldBoundsRadius );
}


//---------------------------------------------------------------------------------------------------------
float Collider2D::GetBounceWith( Collider2D const* other ) const
{
	float myBounce = GetPhysicsMaterialBounciness();
	float otherBounce = other->GetPhysicsMaterialBounciness();

	return myBounce * otherBounce;
}


//---------------------------------------------------------------------------------------------------------
float Collider2D::GetFrictionWith( Collider2D const* other ) const
{
	float myFriction = GetPhysicsMaterialFriction();
	float otherFriction = other->GetPhysicsMaterialFriction();

	return myFriction * otherFriction;
}


//---------------------------------------------------------------------------------------------------------
float Collider2D::GetPhysicsMaterialBounciness() const
{
	return m_physicsMaterial->GetBounciness();
}


//---------------------------------------------------------------------------------------------------------
float Collider2D::GetPhysicsMaterialFriction() const
{
	return m_physicsMaterial->GetFriction();
}


//---------------------------------------------------------------------------------------------------------
Collider2D::~Collider2D()
{
	m_rigidbody = nullptr;
	m_physicsSystem = nullptr;
}

