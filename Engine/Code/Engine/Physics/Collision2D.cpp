#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Core/EngineCommon.hpp"


//---------------------------------------------------------------------------------------------------------
Collision2D::Collision2D( Collider2D* thisC, Collider2D* otherC, Manifold2* manifold )
{
	thisCollider = thisC;
	otherCollider = otherC;

	collisionData = manifold;
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
