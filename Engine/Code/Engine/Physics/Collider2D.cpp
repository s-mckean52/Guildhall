#include "Engine/Physics/Collider2D.hpp"


//---------------------------------------------------------------------------------------------------------
Collider2D::~Collider2D()
{
	m_rigidbody = nullptr;
	m_physicsSystem = nullptr;
}

