#include "Game/Entity.hpp"


//---------------------------------------------------------------------------------------------------------
Entity::Entity( Game* theGame )
{
	m_theGame = theGame;
}


//---------------------------------------------------------------------------------------------------------
Entity::Entity( Game* theGame, float movementSpeed, float attacksPerSecond, int attackDamage, float critChanceFraction )
{
	m_theGame = theGame;
	m_movementSpeedPerSecond = movementSpeed;
	m_attacksPerSecond = attacksPerSecond;
	m_attackDamage = attackDamage;
	m_critChanceFraction = critChanceFraction;
}


//---------------------------------------------------------------------------------------------------------
Entity::~Entity()
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::SetCurrentPosition( Vec2 const& position )
{
	m_currentPosition = position;
}


//---------------------------------------------------------------------------------------------------------
int Entity::GetCritDamage() const
{
	return static_cast<int>( m_attackDamage * m_critDamageMultiplier );
}


