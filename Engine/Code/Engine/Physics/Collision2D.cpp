#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Collision2D::Collision2D( Collider2D* thisC, Collider2D* otherC, Manifold2* manifold, uint toFrameIndex )
{
	frameIndex = toFrameIndex;
	thisCollider = thisC;
	otherCollider = otherC;
	collisionData = manifold;

	uint thisID = thisC->m_id;
	uint otherID = otherC->m_id;
	collisionID = IntVec2( Min( thisID, otherID ), Max( thisID, otherID ) );
}


//---------------------------------------------------------------------------------------------------------
Collision2D::~Collision2D()
{
	delete collisionData;
	collisionData = nullptr;
}


//---------------------------------------------------------------------------------------------------------
Vec2 Collision2D::GetCollisionEdgeCenter() const
{
	Vec2 edgeStart = GetContactEdgeStart();
	Vec2 edgeEnd = GetContactEdgeEnd();
	return ( edgeEnd + edgeStart ) * 0.5f; 
}


//---------------------------------------------------------------------------------------------------------
void Manifold2::SetContactEdge( Vec2 const& startPosition, Vec2 const& endPosition )
{
	collisionEdgeStart = startPosition;
	collisionEdgeEnd = endPosition;
}
