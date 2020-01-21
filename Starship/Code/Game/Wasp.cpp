#include "Game/Wasp.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/PlayerShip.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

//---------------------------------------------------------------------------------------------------------
Wasp::Wasp( Game* theGame, Vec2 position )
	: Entity( theGame, position )
{
	m_acceleration = Vec2( WASP_ACCELERATION, 0.f );
	m_physicsRadius = WASP_PHYSICS_RADIUS;
	m_cosmeticRadius = WASP_COSMETIC_RADIUS;
	m_health = WASP_MAX_HEALTH;
	m_color = WASP_COLOR;
	m_numberOfDebris = 4;
}


//---------------------------------------------------------------------------------------------------------
void Wasp::FaceNearestPlayer()
{
	PlayerShip* playerShip = m_game->m_playerShip;
	if( !playerShip->IsDead() )
	{
		Vec2 playerPos = playerShip->GetPosition();
		m_orientationDegrees = GetAngleToPointDegrees( m_position, playerPos );
		m_acceleration.SetAngleDegrees( m_orientationDegrees );
	}
}


//---------------------------------------------------------------------------------------------------------
void Wasp::Render() const
{
	Rgba8 tailTipColor = RGBA8_RED;
	Rgba8 tailBaseColor( 255, 140, 0 );

	Vertex_PCU waspVerticies[] =
	{
		//Body Left
		Vertex_PCU( Vec2( 2.f, 0.f ),	m_color ),
		Vertex_PCU( Vec2( -2.f, 1.f ),	m_color ),
		Vertex_PCU( Vec2( -1.f, 0.f ),	m_color ),

		//Body Right
		Vertex_PCU( Vec2( 2.f, 0.f ),	m_color ),
		Vertex_PCU( Vec2( -1.f, 0.f ),	m_color ),
		Vertex_PCU( Vec2( -2.f, -1.f ),	m_color ),

		//Thrust
		Vertex_PCU( Vec2( -1.f, 0.f ), tailBaseColor ),
		Vertex_PCU( Vec2( -1.5f, 0.5f ), tailBaseColor ),
		Vertex_PCU( Vec2( -1.5f, -0.5f ), tailBaseColor ),

		Vertex_PCU( Vec2( -1.5f, 0.5f ), tailBaseColor ),
		Vertex_PCU( Vec2( -4.f + m_thrustLengthAddition, 0.f ), tailBaseColor ),
		Vertex_PCU( Vec2( -1.5f, -0.5f ), tailBaseColor ),
	};

	constexpr int BULLET_VERT_SIZE = sizeof( waspVerticies ) / sizeof( waspVerticies[0] );

	TransformVertexArray( BULLET_VERT_SIZE, waspVerticies, m_scale, m_orientationDegrees, m_position );

	g_theRenderer->DrawVertexArray( BULLET_VERT_SIZE, waspVerticies );

	if( m_game->GetGameState() != DEBUG_STATE ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void Wasp::Update( float deltaSeconds )
{
	FaceNearestPlayer();

	if( !IsOffScreen() || !m_game->m_playerShip->IsDead() )
	{
		Entity::Update( deltaSeconds );
	}

	m_thrustLengthAddition = g_RNG->GetRandomFloatZeroToOneInclusive();

	m_velocity.ClampLength( WASP_MAX_SPEED );
}


//---------------------------------------------------------------------------------------------------------
void Wasp::Die()
{
	Entity::Die();

	m_game->DecrementEnemyCount();
}
