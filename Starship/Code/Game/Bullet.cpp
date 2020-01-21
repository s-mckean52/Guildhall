#include "Game/Bullet.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/Game.hpp"


//---------------------------------------------------------------------------------------------------------
Bullet::Bullet( Game *theGame, Vec2 position, float orientation )
	: Entity( theGame, position)
{
	m_orientationDegrees = orientation;
	m_cosmeticRadius = BULLET_COSMETIC_RADIUS;
	m_physicsRadius = BULLET_PHYSICS_RADIUS;
	m_velocity = Vec2( BULLET_SPEED, 0.f );
	m_velocity.RotateDegrees( orientation );
	m_health = BULLET_MAX_HEALTH;
	m_color = RGBA8_RED;
}


//---------------------------------------------------------------------------------------------------------
void Bullet::Render() const
{
	Vertex_PCU bulletVerticies[] = 
	{
		//Head
		Vertex_PCU( Vec2( 0.f, -0.5f ), RGBA8_RED ),
		Vertex_PCU( Vec2( 0.5f, 0.f ), RGBA8_YELLOW ),
		Vertex_PCU( Vec2( 0.f, 0.5f ), RGBA8_RED),

		//Tail
		Vertex_PCU( Vec2( 0.f, -0.5f ), RGBA8_RED ),
		Vertex_PCU( Vec2( 0.f, 0.5f ), RGBA8_RED ),
		Vertex_PCU( Vec2( -2.f, 0.f ), RGBA8_TRANSPARENT_RED )
	};

	constexpr int BULLET_VERT_SIZE = sizeof( bulletVerticies ) / sizeof( bulletVerticies[0] );

	TransformVertexArray( BULLET_VERT_SIZE, bulletVerticies, m_scale, m_orientationDegrees, m_position );

	g_theRenderer->DrawVertexArray( BULLET_VERT_SIZE, bulletVerticies );

	if( m_game->GetGameState() != DEBUG_STATE ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void Bullet::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );

	if( IsOffScreen() ) Die();
}
