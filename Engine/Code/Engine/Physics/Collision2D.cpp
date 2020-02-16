#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Physics/Collider2D.hpp"


//---------------------------------------------------------------------------------------------------------
Collision2D::Collision2D( Collider2D* thisC, Collider2D* otherC )
{
	thisCollider = thisC;
	otherCollider = otherC;

	collisionData = new Manifold2();
}