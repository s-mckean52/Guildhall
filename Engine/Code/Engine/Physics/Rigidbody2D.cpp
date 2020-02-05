#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/Physics2D.hpp"


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::Destroy()
{
	m_physicsSystem->DestroyRigidbody2D( this );
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::TakeCollider( Collider2D* collider )
{
	if( m_collider != nullptr )
	{
		m_physicsSystem->DestroyCollider2D( m_collider );
	}

	m_collider = collider;
	m_collider->m_rigidbody = this;
	m_collider->UpdateWorldShape();
}


//---------------------------------------------------------------------------------------------------------
void Rigidbody2D::SetPosition( Vec2 position )
{
	m_worldPosition = position;
}


//---------------------------------------------------------------------------------------------------------
Rigidbody2D::~Rigidbody2D()
{
	GUARANTEE_OR_DIE( m_collider == nullptr, "Collider was not destroyed before rigidbody delete" );
}

