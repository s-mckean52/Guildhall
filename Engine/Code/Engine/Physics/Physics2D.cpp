#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Collision2D.hpp"

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
	ClearFrameData();

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
	DetectCollisions();
	ResolveCollisions();
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ApplyEffectors( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	SetSceneGravity();
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::MoveRigidbodies( float deltaSeconds )
{
	for( int rbIndex = 0; rbIndex < m_rigidbodies2D.size(); ++rbIndex )
	{
		Rigidbody2D* rb = m_rigidbodies2D[ rbIndex ];
		if( rb && rb->IsSimulated() )
		{
			EulerStep( deltaSeconds, rb );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::DetectCollisions()
{
	for( int thisColliderIndex = 0; thisColliderIndex < m_colliders2D.size(); ++thisColliderIndex )
	{
		Collider2D* thisCollider = m_colliders2D[ thisColliderIndex ];
		for( int otherColliderIndex = thisColliderIndex + 1; otherColliderIndex < m_colliders2D.size(); ++otherColliderIndex )
		{
			Manifold2* newManifold = new Manifold2();
			Collider2D* otherCollider = m_colliders2D[ otherColliderIndex ];
			if( thisCollider->GetManifold( otherCollider, newManifold ) )
			{
				if( thisCollider->m_rigidbody->m_simulationMode == SIMULATION_MODE_STATIC && 
					otherCollider->m_rigidbody->m_simulationMode == SIMULATION_MODE_STATIC ) continue;

				Collision2D* newCollision = new Collision2D( thisCollider, otherCollider, newManifold );
				m_frameCollisions.push_back( newCollision );
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ResolveCollisions()
{
	for( int collisionIndex = 0; collisionIndex < m_frameCollisions.size(); ++collisionIndex )
	{
		Collision2D* currentCollision = m_frameCollisions[ collisionIndex ];
		ResolveCollision( *currentCollision );
	}

	m_frameCollisions.clear();
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ResolveCollision( Collision2D const& collision )
{
	float myMass = collision.thisCollider->GetMass();
	float theirMass = collision.otherCollider->GetMass();
	float pushOnMe = theirMass / ( myMass + theirMass );
	float pushOnThem = 1.0f - pushOnMe;

	SimulationMode myMode = collision.thisCollider->m_rigidbody->m_simulationMode;
	SimulationMode theirMode = collision.otherCollider->m_rigidbody->m_simulationMode;

	if( myMode == SIMULATION_MODE_DYNAMIC && ( theirMode == SIMULATION_MODE_KINEMATIC || theirMode == SIMULATION_MODE_STATIC ) )
	{
		pushOnMe = 1.f;
		pushOnThem = 0.f;
	}
	else if( myMode == SIMULATION_MODE_KINEMATIC && theirMode == SIMULATION_MODE_STATIC )
	{
		pushOnMe = 1.f;
		pushOnThem = 0.f;
	}

	collision.thisCollider->Move( pushOnMe * collision.GetNormal() * collision.GetPenetration() );
	collision.otherCollider->Move( -pushOnThem * collision.GetNormal() * collision.GetPenetration() );

	ApplyImpulseOnCollision( collision );
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
void Physics2D::ApplyImpulseOnCollision( Collision2D const& collision )
{
	Collider2D* me = collision.thisCollider;
	Collider2D* them = collision.otherCollider;

	float myMass = me->GetMass();
	float theirMass = them->GetMass();

	Vec2 myVelocity = me->GetVelocity();
	Vec2 theirVelocity = them->GetVelocity();

	float coefficientOfRestitution = 0; // me->GetBounceWith( them );

	float impulseConstant = 1 + coefficientOfRestitution;
	if( them->m_rigidbody->m_simulationMode != SIMULATION_MODE_DYNAMIC  )
	{
		Vec2 impulse = impulseConstant * ( theirVelocity - myVelocity );
		me->m_rigidbody->ApplyImpulseAt( Vec2(), impulse );
	}
	else if( me->m_rigidbody->m_simulationMode != SIMULATION_MODE_DYNAMIC )
	{
		Vec2 impulse = impulseConstant * ( theirVelocity - myVelocity );
		them->m_rigidbody->ApplyImpulseAt( Vec2(), -impulse );
	}
	else
	{
		impulseConstant *= ( ( myMass * theirMass ) / ( myMass + theirMass ) );
		Vec2 impulse = impulseConstant * ( theirVelocity - myVelocity );
		
		me->m_rigidbody->ApplyImpulseAt( Vec2(), impulse );
		them->m_rigidbody->ApplyImpulseAt( Vec2(), -impulse );
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::SetSceneGravity()
{
	for( int rbIndex = 0; rbIndex < m_rigidbodies2D.size(); ++rbIndex )
	{
		Rigidbody2D* rb = m_rigidbodies2D[ rbIndex ];
		if( rb && rb->DoesTakeForces() )
		{
			rb->AddForceFromAcceleration( m_gravityAcceleration );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ClearFrameData()
{
	for( int rbIndex = 0; rbIndex < m_rigidbodies2D.size(); ++rbIndex )
	{
		Rigidbody2D* rb = m_rigidbodies2D[ rbIndex ];
		if( rb )
		{
			rb->m_frameForces = Vec2();
			rb->m_positionLastFrame = rb->m_worldPosition;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::AddGravityInDownDirection( float gravityToAdd )
{
	m_gravityAcceleration += Vec2( 0.0f, gravityToAdd );
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
	newDiscCollider->m_physicsMaterial = new PhysicsMaterial();
	return (DiscCollider2D*)AddColliderToVector( newDiscCollider );
}


//---------------------------------------------------------------------------------------------------------
PolygonCollider2D* Physics2D::CreatePolygonCollider2D( std::vector<Vec2> polygonVerts, Vec2 localPosition )
{
	PolygonCollider2D* newPolygonCollider = new PolygonCollider2D();
	newPolygonCollider->SetMembers( this, &polygonVerts[ 0 ], static_cast<unsigned int>( polygonVerts.size() ), localPosition );
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
