#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"


//---------------------------------------------------------------------------------------------------------
Physics2D::Physics2D()
{

}


//---------------------------------------------------------------------------------------------------------
Physics2D::~Physics2D()
{

}


//---------------------------------------------------------------------------------------------------------
void Physics2D::BeginFrame()
{

}


//---------------------------------------------------------------------------------------------------------
void Physics2D::Update()
{

}


//---------------------------------------------------------------------------------------------------------
void Physics2D::EndFrame()
{
	for( int rbToBeDestroyedIndex = 0; rbToBeDestroyedIndex < m_rigidbodies2DToBeDestroyed.size(); ++rbToBeDestroyedIndex )
	{
		delete m_rigidbodies2DToBeDestroyed[ rbToBeDestroyedIndex ];
		m_rigidbodies2DToBeDestroyed[ rbToBeDestroyedIndex ] = nullptr;
	}

	for( int colliderToBeDestroyedIndex = 0; colliderToBeDestroyedIndex < m_colliders2DToBeDestroyed.size(); ++colliderToBeDestroyedIndex )
	{
		delete m_colliders2DToBeDestroyed[ colliderToBeDestroyedIndex ];
		m_colliders2DToBeDestroyed[ colliderToBeDestroyedIndex ] = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
Rigidbody2D* Physics2D::CreateRigidbody2D()
{
	Rigidbody2D* newRigidbody2D = new Rigidbody2D();
	newRigidbody2D->m_physicsSystem = this;
	for( int rbIndex = 0; rbIndex < m_rigidbodies2D.size(); ++rbIndex )
	{
		Rigidbody2D* currentRb = m_rigidbodies2D[ rbIndex ];
		if( currentRb == nullptr )
		{
			currentRb = newRigidbody2D;
			return newRigidbody2D;
		}
	}

	m_rigidbodies2D.push_back( newRigidbody2D );
	return newRigidbody2D;
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::DestroyRigidbody2D( Rigidbody2D* rigidbody )
{
	Collider2D* collider = rigidbody->m_collider;
	if( collider != nullptr )
	{
		rigidbody->m_collider = nullptr;
		collider->Destroy();
	}
	m_rigidbodies2DToBeDestroyed.push_back( rigidbody );
}


//---------------------------------------------------------------------------------------------------------
DiscCollider2D* Physics2D::CreateDiscCollider2D( Vec2 localPosition, float radius )
{
	DiscCollider2D* newDiscCollider = new DiscCollider2D();
	newDiscCollider->m_radius = radius;
	newDiscCollider->m_localPosition = localPosition;
	newDiscCollider->m_physicsSystem = this;
	for( int colliderIndex = 0; colliderIndex < m_colliders2D.size(); ++colliderIndex )
	{
		Collider2D* currentCollider = m_colliders2D[ colliderIndex ];
		if( currentCollider == nullptr )
		{
			currentCollider = newDiscCollider;
			return newDiscCollider;
		}
	}

	m_colliders2D.push_back( newDiscCollider );
	return newDiscCollider;
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::DestroyCollider2D( Collider2D* collider )
{
	m_colliders2DToBeDestroyed.push_back( collider );
}
