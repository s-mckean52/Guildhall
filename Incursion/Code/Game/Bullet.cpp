#include "Game/Bullet.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Game/Map.hpp"


//---------------------------------------------------------------------------------------------------------
Bullet::Bullet( Map* theMap, Vec2 position, EntityFaction faction )
	: Entity( theMap, position, faction )
{
	m_isHitByBullet			= false;
	m_pushedByOtherEntities = false;
	m_pushedByWalls			= false;
	m_pushesOtherEntities	= false;

	m_cosmeticRadius		= BULLET_COSMETIC_RADIUS;
	m_physicsRadius			= BULLET_PHYSICS_RADIUS;

	m_velocity	= Vec2( BULLET_VELOCITY, 0.f );
	m_spriteBox = AABB2( Vec2( -0.05f, -0.05f ), Vec2( 0.05f, 0.05f ) );

	if( m_faction == FACTION_GOOD )
	{
		m_entityType = ENTITY_TYPE_GOOD_BULLET;
	}
	else
	{
		m_entityType = ENTITY_TYPE_BAD_BULLET;
	}
}

void Bullet::Render() const
{
	std::vector< Vertex_PCU > bulletVerts;

	AppendVertsForAABB2D( bulletVerts, m_spriteBox, m_color, Vec2( 0.f, 0.f ), Vec2( 1.f, 1.f ) );

	TransformVertexArray( static_cast<int>(bulletVerts.size()), &bulletVerts[0], 1.f, m_orientationDegrees, m_position );

	Texture* bulltTexture = g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Bullet.png" );
	g_theRenderer->BindTexture( bulltTexture );
	g_theRenderer->DrawVertexArray( bulletVerts );

	if( !g_isDebugDraw ) return;

	DebugDraw();
}

void Bullet::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );
}


void Bullet::Die()
{
	m_map->SpawnExplosion( m_position, BULLET_EXPLOSION_RADIUS , BULLET_EXPLOSION_DURATION );
	Entity::Die();
}
