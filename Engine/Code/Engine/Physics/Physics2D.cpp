#include "Engine/Physics/Physics2D.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Physics/DiscCollider2D.hpp"
#include "Engine/Physics/PolygonCollider2D.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"
#include "Engine/Physics/Collision2D.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Math/Vec3.hpp"


//---------------------------------------------------------------------------------------------------------
Physics2D::Physics2D( Clock* gameClock )
{
	SetClock( gameClock );
	m_stepTimer.SetSeconds( gameClock, m_fixedDeltaTime );

	for( uint layerIndex = 0; layerIndex < 32; ++layerIndex )
	{
		m_layerInteractions[ layerIndex ] = 0xffffffff;
	}
}


//---------------------------------------------------------------------------------------------------------
Physics2D::~Physics2D()
{
	for( int frameCollisionIndex = 0; frameCollisionIndex < m_frameCollisions.size(); ++frameCollisionIndex )
	{
		delete m_frameCollisions[frameCollisionIndex];
		m_frameCollisions[frameCollisionIndex] = nullptr;
	}

	for( int frameTriggerCollisionIndex = 0; frameTriggerCollisionIndex < m_frameTriggerCollisions.size(); ++frameTriggerCollisionIndex )
	{
		delete m_frameTriggerCollisions[frameTriggerCollisionIndex];
		m_frameTriggerCollisions[frameTriggerCollisionIndex] = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::BeginFrame()
{

}


//---------------------------------------------------------------------------------------------------------
void Physics2D::Update()
{
	int numberOfSteps = m_stepTimer.CheckAndDecrementAll();

	for( int stepCount = 0; stepCount < numberOfSteps; ++stepCount )
	{
		AdvanceSimulation( static_cast<float>( m_fixedDeltaTime ) );
		++m_currentFrameIndex;
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::EndFrame()
{
	ClearFrameData();

	for( int rbToBeDestroyedIndex = 0; rbToBeDestroyedIndex < m_rigidbodies2D.size(); ++rbToBeDestroyedIndex )
	{
		Rigidbody2D* rb = m_rigidbodies2D[ rbToBeDestroyedIndex ];
		if( rb && rb->IsMarkedForDestroy() )
		{
			delete m_rigidbodies2D[ rbToBeDestroyedIndex ];
			m_rigidbodies2D[ rbToBeDestroyedIndex ] = nullptr;
		}
	}

	for( int colliderToBeDestroyedIndex = 0; colliderToBeDestroyedIndex < m_colliders2D.size(); ++colliderToBeDestroyedIndex )
	{
		Collider2D* collider = m_colliders2D[ colliderToBeDestroyedIndex ];
		if( collider && collider->IsMarkedForDestroy() )
		{
			delete m_colliders2D[ colliderToBeDestroyedIndex ];
			m_colliders2D[ colliderToBeDestroyedIndex ] = nullptr;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::AdvanceSimulation( float deltaSeconds )
{
	UpdateFrameStartPositions();
	ApplyEffectors( deltaSeconds );
	MoveRigidbodies( deltaSeconds );
	UpdateVerletVelocities();

	DetectCollisions();
	DetectTriggerCollisons();
	
	CallOnOverlapEvents();
	CallOnTriggerEvents();
	
	ClearLastFrameCollisions();
	
	ResolveCollisions();
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ApplyEffectors( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	SetSceneGravity();
	ApplyDragOnRigidbodies();
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
void Physics2D::UpdateFrameStartPositions()
{
	for( int rbIndex = 0; rbIndex < m_rigidbodies2D.size(); ++rbIndex )
	{
		Rigidbody2D* rb = m_rigidbodies2D[ rbIndex ];
		if( rb != nullptr )
		{
			rb->m_frameStartPosition = rb->m_worldPosition;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::UpdateVerletVelocities()
{
	for( int rbIndex = 0; rbIndex < m_rigidbodies2D.size(); ++rbIndex )
	{
		Rigidbody2D* rb = m_rigidbodies2D[ rbIndex ];
		if( rb != nullptr )
		{
			rb->UpdateVerletVelocity( static_cast<float>( m_fixedDeltaTime ) );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::DetectCollisions()
{
	for( int thisRigidbodyIndex = 0; thisRigidbodyIndex < m_rigidbodies2D.size(); ++thisRigidbodyIndex )
	{
		Rigidbody2D* thisRigidbody = m_rigidbodies2D[ thisRigidbodyIndex ];
		for( int otherRigidbodyIndex = thisRigidbodyIndex + 1; otherRigidbodyIndex < m_rigidbodies2D.size(); ++otherRigidbodyIndex )
		{
			Rigidbody2D* otherRigidbody = m_rigidbodies2D[ otherRigidbodyIndex ];

			if( thisRigidbody == nullptr || otherRigidbody == nullptr ) continue;
			if( !thisRigidbody->IsEnabled() || !otherRigidbody->IsEnabled() ) continue;

			Collider2D* thisCollider = thisRigidbody->m_collider;
			Collider2D* otherCollider = otherRigidbody->m_collider;

			if( thisCollider == nullptr || otherCollider == nullptr ) continue;
			if( thisCollider->m_isTrigger || otherCollider->m_isTrigger ) continue;
			if( !DoLayersInteract( thisCollider->GetLayer(), otherCollider->GetLayer() ) ) continue;

			Manifold2* newManifold = new Manifold2();
			if( thisCollider->GetManifold( otherCollider, newManifold ) )
			{
				if( thisRigidbody->m_simulationMode == SIMULATION_MODE_STATIC && 
					otherRigidbody->m_simulationMode == SIMULATION_MODE_STATIC ) continue;

				Collision2D* newCollision = nullptr;

				if( thisCollider->GetType() == COLLIDER_TYPE_POLYGON2D && otherCollider->GetType() == COLLIDER_TYPE_DISC2D )
				{
					newCollision = new Collision2D( otherCollider, thisCollider, newManifold, m_currentFrameIndex );
				}
				else
				{
					newCollision = new Collision2D( thisCollider, otherCollider, newManifold, m_currentFrameIndex );
				}
				m_frameCollisions.push_back( newCollision );
			}
			else
			{
				delete newManifold;
				newManifold = nullptr;
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::DetectTriggerCollisons()
{
	for( uint colliderIndex = 0; colliderIndex < m_colliders2D.size(); ++colliderIndex )
	{
		Collider2D* thisCollider = m_colliders2D[ colliderIndex ];

		if( thisCollider == nullptr ) continue;
		if( !thisCollider->m_isTrigger ) continue;

		for( uint rigidbodyIndex = 0; rigidbodyIndex < m_rigidbodies2D.size(); ++rigidbodyIndex )
		{
			Rigidbody2D* currentRigidbody = m_rigidbodies2D[ rigidbodyIndex ];
			if( currentRigidbody == nullptr ) continue;

			Collider2D* currentRigidbodyCollider = currentRigidbody->m_collider;
			if( currentRigidbodyCollider == nullptr ) continue;
			if( !DoLayersInteract( thisCollider->GetLayer(), currentRigidbodyCollider->GetLayer() ) ) continue;

			if( !currentRigidbodyCollider->m_isTrigger && currentRigidbodyCollider != thisCollider )
			{
				if( thisCollider->Intersects( currentRigidbodyCollider ) )
				{
					Collision2D* newTriggerCollision = new Collision2D( thisCollider, currentRigidbodyCollider, nullptr, m_currentFrameIndex );
					m_frameTriggerCollisions.push_back( newTriggerCollision );
				}
			}
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::CallOnOverlapEvents()
{
	for( uint collisionIndex = 0; collisionIndex < m_frameCollisions.size(); ++collisionIndex )
	{
		bool didStay = false;
		Collision2D* thisCollision = m_frameCollisions[ collisionIndex ];
		for( uint otherCollisionIndex = collisionIndex + 1; otherCollisionIndex < m_frameCollisions.size(); ++otherCollisionIndex )
		{
			Collision2D* otherCollision = m_frameCollisions[ otherCollisionIndex ];
			if( thisCollision->collisionID == otherCollision->collisionID && thisCollision->frameIndex < m_currentFrameIndex )
			{
				thisCollision->thisCollider->OnOverlapStay( thisCollision );
				thisCollision->otherCollider->OnOverlapStay( thisCollision );
				didStay = true;

				//update this collison to this frame
				delete thisCollision;
				m_frameCollisions[ collisionIndex ] = otherCollision;
				m_frameCollisions.erase( m_frameCollisions.begin() + otherCollisionIndex );
			}
		}

		if( didStay ) continue;

		if( thisCollision->frameIndex == m_currentFrameIndex )
		{
			thisCollision->thisCollider->OnOverlapEnter( thisCollision );
			thisCollision->otherCollider->OnOverlapEnter( thisCollision );
		}
		else
		{
			thisCollision->thisCollider->OnOverlapLeave( thisCollision );
			thisCollision->otherCollider->OnOverlapLeave( thisCollision );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::CallOnTriggerEvents()
{
	for( uint collisionIndex = 0; collisionIndex < m_frameTriggerCollisions.size(); ++collisionIndex )
	{
		bool didStay = false;
		Collision2D* thisCollision = m_frameTriggerCollisions[ collisionIndex ];

		Collider2D* thisCollider = thisCollision->thisCollider;
		Collider2D* otherCollider = thisCollision->otherCollider;
		if( !thisCollider->m_isTrigger && !otherCollider->m_isTrigger )
		{
			continue;
		}

		for( uint otherCollisionIndex = collisionIndex + 1; otherCollisionIndex < m_frameTriggerCollisions.size(); ++otherCollisionIndex )
		{
			Collision2D* otherCollision = m_frameTriggerCollisions[ otherCollisionIndex ];
			if( thisCollision->collisionID == otherCollision->collisionID )
			{
				thisCollider->OnTriggerStay( thisCollision );
				didStay = true;

				//update this collison to this frame
				delete thisCollision;
				m_frameTriggerCollisions[ collisionIndex ] = otherCollision;
				m_frameTriggerCollisions.erase( m_frameTriggerCollisions.begin() + otherCollisionIndex );
			}
		}

		if( didStay ) continue;

		if( thisCollision->frameIndex == m_currentFrameIndex )
		{
			thisCollider->OnTriggerEnter( thisCollision );
		}
		else
		{
			thisCollider->OnTriggerLeave( thisCollision );
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

	if( myMode == SIMULATION_MODE_DYNAMIC && theirMode != SIMULATION_MODE_DYNAMIC )
	{
		pushOnMe = 1.f;
		pushOnThem = 0.f;
	}
	else if( myMode == SIMULATION_MODE_KINEMATIC && theirMode == SIMULATION_MODE_STATIC )
	{
		pushOnMe = 1.f;
		pushOnThem = 0.f;
	}
	else if (myMode == SIMULATION_MODE_STATIC && theirMode == SIMULATION_MODE_KINEMATIC)
	{
		pushOnMe = 0.f;
		pushOnThem = 1.f;
	}
	else if( myMode != SIMULATION_MODE_DYNAMIC && theirMode == SIMULATION_MODE_DYNAMIC )
	{
		pushOnMe = 0.f;
		pushOnThem = 1.f;
	}

	ApplyImpulseOnCollision( collision );

	collision.thisCollider->Move( pushOnMe * collision.GetNormal() * collision.GetPenetration() );
	collision.otherCollider->Move( -pushOnThem * collision.GetNormal() * collision.GetPenetration() );
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ClearLastFrameCollisions()
{
	for( uint frameCollisionIndex = 0; frameCollisionIndex < m_frameCollisions.size(); ++frameCollisionIndex )
	{
		if( m_frameCollisions[ frameCollisionIndex ]->frameIndex != m_currentFrameIndex )
		{
 			delete m_frameCollisions[ frameCollisionIndex ];
			m_frameCollisions.erase( m_frameCollisions.begin() + frameCollisionIndex );
		}
	}

	for( uint frameTriggerCollisionIndex = 0; frameTriggerCollisionIndex < m_frameTriggerCollisions.size(); ++frameTriggerCollisionIndex )
	{
		if( m_frameTriggerCollisions[ frameTriggerCollisionIndex ]->frameIndex != m_currentFrameIndex )
		{
 			delete m_frameTriggerCollisions[ frameTriggerCollisionIndex ];
			m_frameTriggerCollisions.erase( m_frameTriggerCollisions.begin() + frameTriggerCollisionIndex );
		}
	}
}

//---------------------------------------------------------------------------------------------------------
void Physics2D::EulerStep( float deltaSeconds, Rigidbody2D* rb )
{
	//Position Update
	Vec2 acceleration = rb->GetFrameAcceleration();

	Vec2 deltaVelocity = acceleration * deltaSeconds;
	rb->m_velocity += deltaVelocity;

	Vec2 deltaPosition = rb->m_velocity * deltaSeconds;
	Vec2 newWorldPosition = rb->m_worldPosition + deltaPosition;
	rb->SetPosition( newWorldPosition );


	//Rotation Update
	float angularAcceleration = rb->GetFrameAngularAcceleration();

	float deltaAngularVelocity = angularAcceleration * deltaSeconds;
	rb->m_angularVelocity += deltaAngularVelocity;

	float deltaRotation = rb->GetAngularVelocity() * deltaSeconds;
	float newRotationRadians = rb->GetRotationRadians() + deltaRotation;
	rb->SetRotationRadians( newRotationRadians );
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ApplyImpulseOnCollision( Collision2D const& collision )
{
	Vec2 collisionNormal = collision.GetNormal();
	Vec2 collisionTangent = collisionNormal.GetRotated90Degrees();
	Vec2 collisionEdgeStart = collision.GetContactEdgeStart();
	Vec2 collisionEdgeEnd = collision.GetContactEdgeEnd();
	Vec2 collisionEdgeCenter = collision.GetCollisionEdgeCenter();

	Collider2D* me = collision.thisCollider;
	Collider2D* them = collision.otherCollider;

	Vec2 myImpulseApplicationPoint = GetNearestPointOnLineSegment2D( me->m_rigidbody->m_worldPosition, collisionEdgeStart, collisionEdgeEnd );
	Vec2 theirImpulseApplicationPoint = GetNearestPointOnLineSegment2D( them->m_rigidbody->m_worldPosition, collisionEdgeStart, collisionEdgeEnd );

	float myMass = me->GetMass();
	float theirMass = them->GetMass();

	Vec2 myVelocity = me->m_rigidbody->GetImpactVelocityAtPoint( collisionEdgeCenter );
	Vec2 theirVelocity = them->m_rigidbody->GetImpactVelocityAtPoint( collisionEdgeCenter );
	Vec2 differenceInVelocity = theirVelocity - myVelocity;
	float velocityDiffDotCollisionNormal = DotProduct2D( differenceInVelocity, collisionNormal );
	float velocityDiffDotCollisionTangent = DotProduct2D( differenceInVelocity, collisionTangent );

	float myMoment = me->m_rigidbody->GetMoment();
	float theirMoment = them->m_rigidbody->GetMoment();
	Vec2 myDisplacementToContact = collisionEdgeCenter - me->m_rigidbody->m_worldPosition;
	Vec2 theirDisplacementToContact = collisionEdgeCenter - them->m_rigidbody->m_worldPosition;
	Vec2 myDisplacementToContactTangent = myDisplacementToContact.GetRotated90Degrees();
	Vec2 theirDisplacementToContactTangent = theirDisplacementToContact.GetRotated90Degrees();
	
	float myTangentDotNormal = DotProduct2D( myDisplacementToContactTangent, collisionNormal );
	float myTangentDotTangent = DotProduct2D( myDisplacementToContactTangent, collisionTangent );

	float theirTangentDotNormal = DotProduct2D( theirDisplacementToContactTangent, collisionNormal );
	float theirTangentDotTangent = DotProduct2D( theirDisplacementToContactTangent, collisionTangent );
	
	float myRotationalForce = ( ( myTangentDotNormal * myTangentDotNormal ) / myMoment );
	float myTangentRotationalForce = ( ( myTangentDotTangent * myTangentDotTangent) / myMoment);
	float theirRotationalForce = ( ( theirTangentDotNormal * theirTangentDotNormal ) / theirMoment );
	float theirTangentRotationalForce = ( ( theirTangentDotTangent * theirTangentDotTangent ) / theirMoment );

	float coefficientOfRestitution	= me->GetBounceWith( them );
	float frictionalCoefficient		= me->GetFrictionWith( them );

	float normalImpulseConstant = 1 + coefficientOfRestitution;
	float tangentImpulseConstant = 1 + coefficientOfRestitution;
	if( them->m_rigidbody->m_simulationMode != SIMULATION_MODE_DYNAMIC  )
	{
		normalImpulseConstant /= ( 1 / myMass ) + myRotationalForce;
		tangentImpulseConstant /= ( 1 / myMass ) + myTangentRotationalForce;

		float normalImpulse = normalImpulseConstant * velocityDiffDotCollisionNormal;
		me->m_rigidbody->ApplyImpulseAt( myImpulseApplicationPoint, normalImpulse * collisionNormal );

		if( me->m_rigidbody->m_simulationMode == SIMULATION_MODE_DYNAMIC )
		{
			float tangentImpulse = tangentImpulseConstant * velocityDiffDotCollisionTangent;
			me->m_rigidbody->ApplyFrictionAt( myImpulseApplicationPoint, frictionalCoefficient, collisionNormal, normalImpulse, collisionTangent, tangentImpulse );
		}
		//DebugAddWorldArrow( Vec3( collisionEdgeCenter, 0.f ), Vec3( collisionEdgeCenter, 0.f ) + Vec3( collisionNormal, 0.f ) * normalImpulse, Rgba8::GREEN, 0.1f, DEBUG_RENDER_ALWAYS );
	}
	else if( me->m_rigidbody->m_simulationMode != SIMULATION_MODE_DYNAMIC )
	{
		normalImpulseConstant /= ( 1 / theirMass ) + theirRotationalForce;
		tangentImpulseConstant /= ( 1 / theirMass ) + theirTangentRotationalForce;

		float normalImpulse = normalImpulseConstant * velocityDiffDotCollisionNormal;
		them->m_rigidbody->ApplyImpulseAt( theirImpulseApplicationPoint, collisionNormal * -normalImpulse );

		if( them->m_rigidbody->m_simulationMode == SIMULATION_MODE_DYNAMIC )
		{
			float tangentImpulse = tangentImpulseConstant * velocityDiffDotCollisionTangent;
			them->m_rigidbody->ApplyFrictionAt( theirImpulseApplicationPoint, frictionalCoefficient, collisionNormal, normalImpulse, collisionTangent, -tangentImpulse );	
		}
		//DebugAddWorldArrow( Vec3( collisionEdgeCenter, 0.f ), Vec3( collisionEdgeCenter, 0.f ) + Vec3( collisionNormal, 0.f ) * -normalImpulse, Rgba8::GREEN, 0.1f, DEBUG_RENDER_ALWAYS );
	}
	else
	{
		float massRatio = ( 1 / myMass ) + ( 1 / theirMass );
		normalImpulseConstant /= ( massRatio + myRotationalForce + theirRotationalForce );
		tangentImpulseConstant /= ( massRatio + myTangentRotationalForce + theirTangentRotationalForce );

		float normalImpulse = normalImpulseConstant * velocityDiffDotCollisionNormal;
		float tangentImpulse = tangentImpulseConstant * velocityDiffDotCollisionTangent;

		me->m_rigidbody->ApplyImpulseAt( myImpulseApplicationPoint, collisionNormal * normalImpulse );
		them->m_rigidbody->ApplyImpulseAt( theirImpulseApplicationPoint, collisionNormal * -normalImpulse );

		me->m_rigidbody->ApplyFrictionAt( myImpulseApplicationPoint, frictionalCoefficient, collisionNormal, normalImpulse, collisionTangent, tangentImpulse );
		them->m_rigidbody->ApplyFrictionAt( theirImpulseApplicationPoint, frictionalCoefficient, collisionNormal, normalImpulse, collisionTangent, -tangentImpulse );
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ToggleLayerInteraction( uint layerIndexA, uint layerIndexB )
{
	m_layerInteractions[layerIndexA] ^= 1 << layerIndexB;
	if( layerIndexA != layerIndexB )
	{
		m_layerInteractions[layerIndexB] ^= 1 << layerIndexA;
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::DisableLayerInteraction( uint layerIndexA, uint layerIndexB )
{
	if( DoLayersInteract( layerIndexA, layerIndexB ) )
	{
		ToggleLayerInteraction( layerIndexA, layerIndexB );
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::EnableLayerInteraction( uint layerIndexA, uint layerIndexB )
{
	if( !DoLayersInteract( layerIndexA, layerIndexB ) )
	{
		ToggleLayerInteraction( layerIndexA, layerIndexB );
	}
}


//---------------------------------------------------------------------------------------------------------
bool Physics2D::DoLayersInteract( uint layerIndexA, uint layerIndexB )
{
	uint layerOffset = 1 << layerIndexB;
	uint layerAnd = m_layerInteractions[layerIndexA] & layerOffset;
	return ( layerAnd != 0 );
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
void Physics2D::ApplyDragOnRigidbodies()
{
	for( int rbIndex = 0; rbIndex < m_rigidbodies2D.size(); ++rbIndex )
	{
		Rigidbody2D* rb = m_rigidbodies2D[ rbIndex ];
		if( rb != nullptr )
		{
			rb->ApplyDragForce();
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::SetClock( Clock* clock )
{
	m_clock = clock;
	if( clock == nullptr )
	{
		m_clock = Clock::GetMaster();
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::ToggleClockPause()
{
	if( m_clock->IsPaused() )
	{
		m_clock->Resume();
	}
	else
	{
		m_clock->Pause();
	}
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::SetClockScale( double clockScale )
{
	m_clock->SetScale( clockScale );
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::SetFixedDeltaTime( double newFixedDeltaTime )
{
	m_fixedDeltaTime = newFixedDeltaTime;
	m_stepTimer.SetSeconds( newFixedDeltaTime );
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
			rb->m_frameTorque = 0.f;
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
			m_rigidbodies2D[ rbIndex ] = newRigidbody2D;
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
		collider->Destroy();
		rigidbody->m_collider = nullptr;
	}
	rigidbody->MarkForDestroy( true );
}


//---------------------------------------------------------------------------------------------------------
DiscCollider2D* Physics2D::CreateDiscCollider2D( Vec2 localPosition, float radius )
{
	DiscCollider2D* newDiscCollider = new DiscCollider2D();
	newDiscCollider->m_radius = radius;
	newDiscCollider->m_localPosition = localPosition;
	newDiscCollider->m_physicsSystem = this;
	newDiscCollider->m_physicsMaterial = new PhysicsMaterial();
	newDiscCollider->m_id = static_cast<uint>( m_colliders2D.size() );
	return (DiscCollider2D*)AddColliderToVector( newDiscCollider );
}


//---------------------------------------------------------------------------------------------------------
PolygonCollider2D* Physics2D::CreatePolygonCollider2D( std::vector<Vec2> polygonVerts, Vec2 localPosition )
{
	PolygonCollider2D* newPolygonCollider = new PolygonCollider2D();
	newPolygonCollider->SetMembers( this, &polygonVerts[ 0 ], static_cast<unsigned int>( polygonVerts.size() ), localPosition );
	newPolygonCollider->m_id = static_cast<uint>( m_colliders2D.size() );
	return (PolygonCollider2D*)AddColliderToVector( newPolygonCollider );
}


//---------------------------------------------------------------------------------------------------------
void Physics2D::DestroyCollider2D( Collider2D* collider )
{
	collider->MarkForDestroy( true );
}


//---------------------------------------------------------------------------------------------------------
Collider2D* Physics2D::AddColliderToVector( Collider2D* newCollider )
{
	for( int colliderIndex = 0; colliderIndex < m_colliders2D.size(); ++colliderIndex )
	{
		Collider2D* currentCollider = m_colliders2D[ colliderIndex ];
		if( currentCollider == nullptr )
		{
			m_colliders2D[ colliderIndex ] = newCollider;
			return newCollider;
		}
	}
	m_colliders2D.push_back( newCollider );
	return newCollider;
}
