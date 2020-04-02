#pragma once
#include "Engine/Math/Vec2.hpp"

class	Collider2D;
struct	Manifold2;

//---------------------------------------------------------------------------------------------------------
struct Manifold2
{
	//Vec2 contactPosition;
	Vec2 collisionEdgeStart;
	Vec2 collisionEdgeEnd;
	Vec2 collisionNormal;
	float penetrationDistance = 0.f;

public:
	void SetContactEdge( Vec2 const& startPosition, Vec2 const& endPosition );
};


//---------------------------------------------------------------------------------------------------------
struct Collision2D
{
public:
	Collider2D* thisCollider	= nullptr;
	Collider2D* otherCollider	= nullptr;
	Manifold2* collisionData	= nullptr;

public:
	Collision2D( Collider2D* thisC, Collider2D* otherC, Manifold2* manifold );
	~Collision2D();

	Vec2 GetNormal() const			{ return collisionData->collisionNormal; }
	Vec2 GetContactEdgeStart() const	{ return collisionData->collisionEdgeStart; }
	Vec2 GetContactEdgeEnd() const	{ return collisionData->collisionEdgeEnd; }
	float GetPenetration() const	{ return collisionData->penetrationDistance; }
	Vec2 GetCollisionEdgeCenter() const;
};