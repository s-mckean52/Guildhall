#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Game/Game.hpp"


//---------------------------------------------------------------------------------------------------------
Entity::Entity( Map* theMap, const Vec2& startPosition, EntityFaction faction )
	: m_map( theMap )
	, m_position( startPosition )
	, m_faction( faction )
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_orientationDegrees += m_angularVelocity * deltaSeconds;

	m_velocity += m_acceleration.GetRotatedDegrees( m_orientationDegrees ) * deltaSeconds;
	m_position += m_velocity * deltaSeconds;

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
void Entity::SetPosition( const Vec2& newPosition )
{
	m_position = newPosition;
}


//---------------------------------------------------------------------------------------------------------
void Entity::SetOrientationDegrees( float newOrientationDegrees )
{
	m_orientationDegrees = newOrientationDegrees;
	m_velocity.SetAngleDegrees( m_orientationDegrees );
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
	DrawLineBetweenPoints( m_position, m_position + m_velocity, Rgba8::YELLOW, DEBUG_THICKNESS);
	//DrawCircleAtPoint( m_position, m_cosmeticRadius, Rgba8::MAGENTA, DEBUG_THICKNESS );
	DrawAABB2AtPoint( m_position, m_spriteBox, Rgba8::MAGENTA, DEBUG_THICKNESS );
	DrawCircleAtPoint( m_position, m_physicsRadius, Rgba8::CYAN, DEBUG_THICKNESS );
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