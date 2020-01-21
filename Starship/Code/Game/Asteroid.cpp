#include "Game/Asteroid.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
Asteroid::Asteroid( Game* theGame )
	: Entity( theGame, Vec2( 0, 0 ) )
{
	m_numberOfDebris = NUM_ASTEROID_VERTICIES;
	m_color = RGBA8_MEDIUM_DARK_GREY;
	m_health = ASTEROID_MAX_HEALTH;
	m_cosmeticRadius = ASTEROID_COSMETIC_RADIUS;
	m_physicsRadius = ASTEROID_PHYSICS_RADIUS;

	m_angularVelocity = g_RNG->GetRandomFloatInRange( -200.f, 200.f );

	m_position.x = g_RNG->GetRandomFloatInRange( m_cosmeticRadius, CAMERA_SIZE_X - m_cosmeticRadius );
	m_position.y = g_RNG->GetRandomFloatInRange( m_cosmeticRadius, CAMERA_SIZE_Y - m_cosmeticRadius );

	m_velocity.x = g_RNG->GetRandomFloatInRange( -1.f, 1.f );
	m_velocity.y = g_RNG->GetRandomFloatInRange( -1.f, 1.f );

	m_velocity.Normalize();
	m_velocity *= ASTEROID_SPEED;

	CreateVertecies();
}


//---------------------------------------------------------------------------------------------------------
void Asteroid::CreateVertecies()
{
	constexpr float circlePercentage = 360.f / static_cast<float>( NUM_ASTEROID_VERTICIES );

	float currentAngle = 0.f;

	for( int asteroidVertexIndex = 0; asteroidVertexIndex < NUM_ASTEROID_VERTICIES; ++asteroidVertexIndex )
	{
		Vec2 newVertPosition;

		float randomLength = g_RNG->GetRandomFloatInRange( m_physicsRadius, m_cosmeticRadius );

		newVertPosition = newVertPosition.MakeFromPolarDegrees( currentAngle, randomLength );

		m_asteroidVertecies[ asteroidVertexIndex ] = newVertPosition;

		currentAngle += circlePercentage;
	}
}


//---------------------------------------------------------------------------------------------------------
void Asteroid::Render() const
{
	Vertex_PCU worldAsteroidVerticies[ 3 * NUM_ASTEROID_VERTICIES ];

	int worldVertsSize = sizeof( worldAsteroidVerticies ) / sizeof( worldAsteroidVerticies[0] );
	
	int localVertsCount = 0;

	for( int worldVertIndex = 0; worldVertIndex < worldVertsSize; worldVertIndex += 3 )
	{
		Vec2 nextVertPosition;
		Vec2 currentVertPosition = m_asteroidVertecies[ localVertsCount ];

		if( localVertsCount < NUM_ASTEROID_VERTICIES - 1 )
		{
			 nextVertPosition = m_asteroidVertecies[ localVertsCount + 1 ];
		}
		else
		{
			nextVertPosition = m_asteroidVertecies[ 0 ];
		}

		worldAsteroidVerticies[ worldVertIndex ] = Vertex_PCU( Vec2( 0, 0 ), m_color );
		worldAsteroidVerticies[ worldVertIndex + 1 ] = Vertex_PCU( currentVertPosition, m_color );
		worldAsteroidVerticies[ worldVertIndex + 2 ] = Vertex_PCU( nextVertPosition, m_color );

		++localVertsCount;
	}

	TransformVertexArray( worldVertsSize, worldAsteroidVerticies, m_scale, m_orientationDegrees, m_position );

	g_theRenderer->DrawVertexArray( worldVertsSize, worldAsteroidVerticies );

	if( m_game->GetGameState() != DEBUG_STATE ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void Asteroid::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );

	if( IsOffScreen() ) Die();
}


//---------------------------------------------------------------------------------------------------------
void Asteroid::Die()
{
	Entity::Die();

	m_game->DecrementEnemyCount();
}
