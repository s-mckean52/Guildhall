#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


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
bool Collider2D::Intersects( Collider2D const* other ) const
{
	Collider2DType myType = GetType();
	Collider2DType otherType = other->GetType();

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
Collider2D::~Collider2D()
{
	m_rigidbody = nullptr;
	m_physicsSystem = nullptr;
}

