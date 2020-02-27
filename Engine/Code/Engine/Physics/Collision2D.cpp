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
