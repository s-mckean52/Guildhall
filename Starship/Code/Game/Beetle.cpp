#include "Game/Beetle.hpp"
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
Beetle::Beetle( Game* theGame, Vec2 position )
	: Entity( theGame, position )
{
	m_velocity			= Vec2( BEETLE_SPEED, 0.f );
	m_physicsRadius		= BEETLE_PHYSICS_RADIUS;
	m_cosmeticRadius	= BEETLE_COSMETIC_RADIUS;
	m_health			= BEETLE_MAX_HEALTH;
	m_color				= BEETLE_COLOR;
	m_numberOfDebris	= 6;
}


//---------------------------------------------------------------------------------------------------------
void Beetle::FaceNearestPlayer()
{
	PlayerShip *playerShip = m_game->m_playerShip;
	if( !playerShip->IsDead() )
	{
		Vec2 playerPos = playerShip->GetPosition();
		m_orientationDegrees = GetAngleToPointDegrees( m_position, playerPos);
		m_velocity.SetAngleDegrees( m_orientationDegrees );
	}
}


//---------------------------------------------------------------------------------------------------------
void Beetle::Render() const
{
	Rgba8 tailTipColor = RGBA8_RED;
	Rgba8 tailBaseColor( 255, 140, 0 );

	Vertex_PCU beetleVerticies[] = 
	{
		//Body Left
		Vertex_PCU( Vec2( 2.f, 0.f ),	m_color ),
		Vertex_PCU( Vec2( -1.f, 2.f ),	m_color ),
		Vertex_PCU( Vec2( -1.f, 0.f ),	m_color ),

		//Body Right
		Vertex_PCU( Vec2( 2.f, 0.f ),	m_color ),
		Vertex_PCU( Vec2( -1.f, 0.f ),	m_color ),
		Vertex_PCU( Vec2( -1.f, -2.f ),	m_color ),

		//Left Tail
		Vertex_PCU( Vec2( -1.f, 2.f ),	m_color ),
		Vertex_PCU( Vec2( -2.f, 1.f ),	m_color ),
		Vertex_PCU( Vec2( -1.f, 0.f ),	m_color ),

		//Right Tail
		Vertex_PCU( Vec2( -1.f, 0.f ),	m_color ),
		Vertex_PCU( Vec2( -2.f, -1.f ),	m_color ),
		Vertex_PCU( Vec2( -1.f, -2.f ),	m_color ),

		//Thrust
		Vertex_PCU( Vec2( -1.f, 0.f ), tailBaseColor ),
		Vertex_PCU( Vec2( -1.5f, 0.5f ), tailBaseColor ),
		Vertex_PCU( Vec2( -1.5f, -0.5f ), tailBaseColor ),

		Vertex_PCU( Vec2( -1.5f, 0.5f ), tailBaseColor ),
		Vertex_PCU( Vec2( -3.f + m_thrustLengthAddition, 0.f ), tailBaseColor ),
		Vertex_PCU( Vec2( -1.5f, -0.5f ), tailBaseColor ),
	};

	constexpr int BULLET_VERT_SIZE = sizeof( beetleVerticies ) / sizeof( beetleVerticies[0] );

	TransformVertexArray( BULLET_VERT_SIZE, beetleVerticies, m_scale, m_orientationDegrees, m_position );

	g_theRenderer->DrawVertexArray( BULLET_VERT_SIZE, beetleVerticies );

	if( m_game->GetGameState() != DEBUG_STATE ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void Beetle::Update( float deltaSeconds )
{
	FaceNearestPlayer();

	if( !IsOffScreen() || !m_game->m_playerShip->IsDead() )
	{
		Entity::Update( deltaSeconds );
	}

	m_thrustLengthAddition = g_RNG->GetRandomFloatZeroToOneInclusive();
}


//---------------------------------------------------------------------------------------------------------
void Beetle::Die()
{
	Entity::Die();

	m_game->DecrementEnemyCount();
}
