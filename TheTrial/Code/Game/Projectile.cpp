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
	m_physicsRadius = 0.05f;
	m_targetEntity = target;
}


//---------------------------------------------------------------------------------------------------------
Projectile::Projectile( Game* theGame, int damage, float movementSpeed, Vec2 const& direction, float range )
	: Entity( theGame )
{
	m_damage = damage;
	m_movementSpeed = movementSpeed;
	m_physicsRadius = 0.05f;

	m_rangeRemaining = range;
	m_direction = direction.GetNormalized();
}


//---------------------------------------------------------------------------------------------------------
Projectile::~Projectile()
{
}


//---------------------------------------------------------------------------------------------------------
void Projectile::Update( float deltaSeconds )
{
	if( m_isDead )
		return;

	if( m_targetEntity != nullptr && DoDiscsOverlap( m_currentPosition, m_physicsRadius, m_targetEntity->GetCurrentPosition(), m_targetEntity->GetPhysicsRadius() ) )
	{
		DealDamageToActor( m_targetEntity );
		return;
	}
	
	Vec2 displacementToDestination;
	float displacementProjectedDistance = 1000.f;
	if( m_targetEntity != nullptr )
	{
		Vec2 positionToMoveTo = m_targetEntity->GetCurrentPosition();
		displacementToDestination = positionToMoveTo - m_currentPosition;
		m_direction = displacementToDestination.GetNormalized();
		displacementProjectedDistance = GetProjectedLength2D( displacementToDestination, m_direction );
	}
	else
	{
		m_rangeRemaining -= m_movementSpeed * deltaSeconds;
		if( m_rangeRemaining <= 0.f )
		{
			m_isDead = true;
		}
	}
	
	Vec2 movementVector = m_direction * m_movementSpeed * deltaSeconds;
	float movementProjectedDistance = GetProjectedLength2D( movementVector, m_direction );
	
	if( displacementProjectedDistance < movementProjectedDistance )
	{
		movementVector = displacementToDestination;
	}
	m_currentPosition += movementVector;
}


//---------------------------------------------------------------------------------------------------------
void Projectile::Render() const
{
	if( m_isDead )
		return;

	DrawCircleAtPoint( m_currentPosition, m_physicsRadius, Rgba8::ORANGE, 0.1f );
}


//---------------------------------------------------------------------------------------------------------
bool Projectile::HasTarget() const
{
	return m_targetEntity != nullptr;
}


//---------------------------------------------------------------------------------------------------------
void Projectile::DealDamageToActor( Actor* actorToDealDamageTo )
{
	m_isDead = true;
	actorToDealDamageTo->TakeDamage( m_damage );
}