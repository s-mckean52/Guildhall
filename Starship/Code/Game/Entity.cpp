#include "Game/Entity.hpp"

class Game;

//---------------------------------------------------------------------------------------------------------
Entity::Entity( Game* theGame, const Vec2& startPosition )
	: m_game( theGame )
	, m_position( startPosition )
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_position += m_velocity * deltaSeconds;
	m_velocity += m_acceleration * deltaSeconds;

	m_orientationDegrees += m_angularVelocity * deltaSeconds;
	
	if( m_health <= 0 )
	{
		Die();
	}
}


//---------------------------------------------------------------------------------------------------------
void Entity::Render() const
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::Die()
{
	m_isDead = true;
	m_isGarbage = true;
}


//---------------------------------------------------------------------------------------------------------
void Entity::TakeDamage( int damageDealt )
{
	m_health -= damageDealt;
	if( m_health <= 0 )
	{
		Die();
	}
}


//---------------------------------------------------------------------------------------------------------
void Entity::DebugDraw() const
{
	DrawLineBetweenPoints( m_position, m_position + m_velocity, RGBA8_YELLOW, 0.2f );
	DrawCircleAtPoint( m_position, m_cosmeticRadius, RGBA8_MAGENTA, 0.2f );
	DrawCircleAtPoint( m_position, m_physicsRadius, RGBA8_CYAN, 0.2f );
}


//---------------------------------------------------------------------------------------------------------
bool Entity::IsOffScreen() const
{
	if( m_position.x <= -m_cosmeticRadius || m_position.x >= CAMERA_SIZE_X + m_cosmeticRadius )
	{
		return true;
	}
	else if( m_position.y <= -m_cosmeticRadius || m_position.y >= CAMERA_SIZE_Y + m_cosmeticRadius )
	{
		return true;
	}
	return false;
}
