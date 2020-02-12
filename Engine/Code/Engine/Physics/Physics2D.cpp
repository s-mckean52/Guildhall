#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
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
void Physics2D::Update( float deltaSeconds )
{
	AdvanceSimulation( deltaSeconds );
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
void Physics2D::AdvanceSimulation( float deltaSeconds )
{
	ApplyEffectors( deltaSeconds );
	MoveRigidbodies( deltaSeconds );
	//Clean Up is end frame
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ApplyEffectors( float deltaSeconds )
{
	
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::MoveRigidbodies( float deltaSeconds )
{
	for( int rbIndex = 0; rbIndex < m_rigidbodies2D.size(); ++rbIndex )
	{
		Rigidbody2D* rb = m_rigidbodies2D[ rbIndex ];
		if( rb->DoesMove() )
		{
			rb->EulerStep( deltaSeconds, rb );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::EulerStep( float deltaSeconds, Rigidbody2D* rb )
{
	Vec2 acceleration = rb->GetFrameAcceleration();

	Vec2 deltaVelocity = acceleration * deltaSeconds;
	rb->m_velocity += deltaVelocity;

	Vec2 deltaPosition = rb->m_velocity * deltaSeconds;
	rb->SetPosition( rb->m_worldPosition + deltaPosition );
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
	return (DiscCollider2D*)AddColliderToVector( newDiscCollider );
}


//---------------------------------------------------------------------------------------------------------
PolygonCollider2D* Physics2D::CreatePolygonCollider2D( std::vector<Vec2> polygonVerts, Vec2 localPosition )
{
	PolygonCollider2D* newPolygonCollider = new PolygonCollider2D();
	newPolygonCollider->SetMembers( this, &polygonVerts[ 0 ], polygonVerts.size(), localPosition );
	return (PolygonCollider2D*)AddColliderToVector( newPolygonCollider );
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::DestroyCollider2D( Collider2D* collider )
{
	m_colliders2DToBeDestroyed.push_back( collider );
}


//---------------------------------------------------------------------------------------------------------
Collider2D* Physics2D::AddColliderToVector( Collider2D* newCollider )
{
	for( int colliderIndex = 0; colliderIndex < m_colliders2D.size(); ++colliderIndex )
	{
		Collider2D* currentCollider = m_colliders2D[colliderIndex];
		if( currentCollider == nullptr )
		{
			currentCollider = newCollider;
			return newCollider;
		}
	}
	m_colliders2D.push_back( newCollider );
	return newCollider;
}
