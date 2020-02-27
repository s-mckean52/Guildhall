#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"


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
	UNUSED( col0 );
	UNUSED( col1 );
// 	PolygonCollider2D const* polygon0 = (PolygonCollider2D*)col0;
// 	PolygonCollider2D const* polygon1 = (PolygonCollider2D*)col1;

	return false;
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
		manifold->contactPosition = contactPoint;
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
		manifold->contactPosition = closestPointOnPolygonEdge;
		manifold->collisionNormal = normal;
		manifold->penetrationDistance = penetration;
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------------------
static bool PolygonVPolygonManifoldGeneration( Collider2D const* col0, Collider2D const* col1, Manifold2* manifold )
{
	UNUSED( col0 );
	UNUSED( col1 );
	UNUSED( manifold );
// 	PolygonCollider2D const* polygon0 = (PolygonCollider2D*)col0;
// 	PolygonCollider2D const* polygon1 = (PolygonCollider2D*)col1;

	return false;
}


//---------------------------------------------------------------------------------------------------------
static manifold_check_cb s_manifoldCheck[NUM_COLLIDER_TYPE * NUM_COLLIDER_TYPE] ={
	/*				disc,							polygon		*/
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
}


//---------------------------------------------------------------------------------------------------------
float Collider2D::GetBounceWith( Collider2D const* other ) const
{
	float myBounce = GetPhysicsMaterialBounciness();
	float otherBounce = other->GetPhysicsMaterialBounciness();

	return myBounce * otherBounce;
}


//---------------------------------------------------------------------------------------------------------
float Collider2D::GetPhysicsMaterialBounciness() const
{
	return m_physicsMaterial->GetBounciness();
}


//---------------------------------------------------------------------------------------------------------
Collider2D::~Collider2D()
{
	m_rigidbody = nullptr;
	m_physicsSystem = nullptr;
}

