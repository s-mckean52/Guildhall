#include "Game/Projectile.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Actor.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Projectile::Projectile( Game* theGame, int damage, float movementSpeed, Actor* target )
	:Entity( theGame )
{
	m_damage = damage;
	m_movementSpeed = movementSpeed;
	m_targetEntity = target;
	m_physicsRadius = 0.05f;
}


//---------------------------------------------------------------------------------------------------------
Projectile::~Projectile()
{
}


//---------------------------------------------------------------------------------------------------------
void Projectile::Update( float deltaSeconds )
{
	if( m_hasHitTarget )
		return;

	if( DoDiscsOverlap( m_currentPosition, m_physicsRadius, m_targetEntity->GetCurrentPosition(), m_targetEntity->GetPhysicsRadius() ) )
	{
		m_hasHitTarget = true;
		m_targetEntity->TakeDamage( m_damage );
		return;
	}

	if( m_targetEntity == nullptr )
	{
		//TODO: behavior for untargeted
		return;
	}
	
	Vec2 positionToMoveTo = m_targetEntity->GetCurrentPosition();
	Vec2 displacementToDestination = positionToMoveTo - m_currentPosition;
	Vec2 directionTowardsDestination = displacementToDestination.GetNormalized();
	Vec2 movementVector = directionTowardsDestination * m_movementSpeed * deltaSeconds;
	
	float displacementProjectedDistance = GetProjectedLength2D( displacementToDestination, directionTowardsDestination);
	float movementProjectedDistance = GetProjectedLength2D( movementVector, directionTowardsDestination );
	
	if( displacementProjectedDistance < movementProjectedDistance )
	{
		movementVector = displacementToDestination;
	}
	m_currentPosition += movementVector;
}


//---------------------------------------------------------------------------------------------------------
void Projectile::Render() const
{
	if( m_hasHitTarget )
		return;

	DrawCircleAtPoint( m_currentPosition, m_physicsRadius, Rgba8::ORANGE, 0.1f );
}
