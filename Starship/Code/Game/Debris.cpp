#include "Game/Debris.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
Debris::Debris( Game* theGame, Vec2 position, Vec2 velocity, Rgba8 color, float cosmeticRadius )
	: Entity( theGame, position )
{
	m_color = color;
	m_color.a = static_cast<unsigned char>( static_cast<float>( m_color.a ) * m_alphaPercent );
	
	m_angularVelocity = g_RNG->GetRandomFloatInRange( 1.f, velocity.GetLength() );

	m_cosmeticRadius = cosmeticRadius;
	m_physicsRadius = cosmeticRadius * 0.75f;
	
	m_velocity.x = g_RNG->GetRandomFloatInRange( -MAX_DEBRIS_SPEED, MAX_DEBRIS_SPEED );
	m_velocity.y = g_RNG->GetRandomFloatInRange( -MAX_DEBRIS_SPEED, MAX_DEBRIS_SPEED );

	float velocityDiectionDegrees = g_RNG->GetRandomFloatInRange( 0.f, 360.f );
	m_velocity.SetAngleDegrees( velocityDiectionDegrees );
	CreateVertices();
}


//---------------------------------------------------------------------------------------------------------
void Debris::Render() const
{
	Rgba8 debrisColor = m_color;
	debrisColor.a = static_cast<unsigned char>( static_cast<float>( debrisColor.a ) * m_alphaPercent );

	Vertex_PCU worldDebrisVerticies[ 3 * NUM_DEBRIS_VERTS ];

	int worldVertsSize = sizeof( worldDebrisVerticies ) / sizeof( worldDebrisVerticies[0] );

	int localVertsCount = 0;

	for( int worldVertIndex = 0; worldVertIndex < worldVertsSize; worldVertIndex += 3 )
	{
		Vec2 nextVertPosition;
		Vec2 currentVertPosition = m_debrisVertices[ localVertsCount ];

		if( localVertsCount < NUM_DEBRIS_VERTS - 1 )
		{
			nextVertPosition = m_debrisVertices[ localVertsCount + 1 ];
		}
		else
		{
			nextVertPosition = m_debrisVertices[ 0 ];
		}

		worldDebrisVerticies[worldVertIndex] = Vertex_PCU( Vec2( 0, 0 ), m_color );
		worldDebrisVerticies[worldVertIndex + 1] = Vertex_PCU( currentVertPosition, m_color );
		worldDebrisVerticies[worldVertIndex + 2] = Vertex_PCU( nextVertPosition, m_color );

		++localVertsCount;
	}

	TransformVertexArray( worldVertsSize, worldDebrisVerticies, m_scale, m_orientationDegrees, m_position );

	g_theRenderer->DrawVertexArray( worldVertsSize, worldDebrisVerticies );

	if( m_game->GetGameState() != DEBUG_STATE ) return;

	DebugDraw();
}


//---------------------------------------------------------------------------------------------------------
void Debris::Update( float deltaSeconds )
{
	Entity::Update( deltaSeconds );

	m_alphaPercent -= DEBRIS_ALPHA_ABBERATION * deltaSeconds;
	
	if( IsOffScreen() || m_alphaPercent <= 0.f ) Die();
}


//---------------------------------------------------------------------------------------------------------
void Debris::CreateVertices()
{
	constexpr float circlePercentage = 360.f / static_cast<float>( NUM_DEBRIS_VERTS );

	float currentAngle = 0.f;

	for( int debrisVertexIndex = 0; debrisVertexIndex < NUM_DEBRIS_VERTS; ++debrisVertexIndex )
	{
		Vec2 newVertPosition;

		float randomLength = g_RNG->GetRandomFloatInRange( m_physicsRadius, m_cosmeticRadius );

		newVertPosition = newVertPosition.MakeFromPolarDegrees( currentAngle, randomLength );

		m_debrisVertices[ debrisVertexIndex ] = newVertPosition;

		currentAngle += circlePercentage;
	}
}