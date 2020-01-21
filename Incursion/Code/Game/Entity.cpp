#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Map.hpp"
#include "Game/Tile.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"


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
void Entity::HealthRegen( float deltaSeconds )
{
	if( m_health < m_maxHealth )
	{
		m_healthRegenTimer += deltaSeconds;
	}

	if( m_healthRegenTimer > HEALTH_REGEN_TIME_SECONDS )
	{
		m_health++;
		m_healthRegenTimer -= HEALTH_REGEN_TIME_SECONDS;
	}
}


//---------------------------------------------------------------------------------------------------------
void Entity::Render() const
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::RenderHealthBar( const Rgba8& healthColor, const Rgba8& backgroundColor ) const
{
	Vec2 healthBarPosition = m_position - Vec2( m_cosmeticRadius, m_cosmeticRadius + HEALTH_BAR_HEIGHT );
	float healthBarWidth = m_cosmeticRadius * 2.f;
	float currentHealthFraction = m_health / static_cast<float>( m_maxHealth );

	std::vector<Vertex_PCU> healthBarBackground;
	AppendVertsForAABB2D( healthBarBackground, AABB2( 0, 0, healthBarWidth, HEALTH_BAR_HEIGHT ), backgroundColor );
	TransformVertexArray( healthBarBackground, 1.f, 0.f, healthBarPosition );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( healthBarBackground );

	std::vector<Vertex_PCU> healthBar;
	AppendVertsForAABB2D( healthBar, AABB2( 0, 0, healthBarWidth * currentHealthFraction, HEALTH_BAR_HEIGHT ), healthColor );
	TransformVertexArray( healthBar, 1.f, 0.f, healthBarPosition );

	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( healthBar );
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
void Entity::SetMap( Map* theMap )
{
	m_map = theMap;
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
	DrawCircleAtPoint( m_position, m_cosmeticRadius, Rgba8::MAGENTA, DEBUG_THICKNESS );
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


//---------------------------------------------------------------------------------------------------------
float Entity::GetSlowFractionOnMud()
{
	TileType occupiedTileType = m_map->GetEntityCurrentTileType( this );

	if( occupiedTileType == TILE_TYPE_MUD )
	{
		return MUD_SLOW_FRACTION;
	}

	return 1.0f;
}
