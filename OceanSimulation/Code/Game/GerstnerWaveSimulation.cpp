#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Game/GerstnerWaveSimulation.hpp"
#include "Game/Game.hpp"

//---------------------------------------------------------------------------------------------------------
GersternWaveSimulation::GersternWaveSimulation( Vec2 const& dimensions, uint samples )
	: WaveSimulation( dimensions, samples, 37.f )
{
}


//---------------------------------------------------------------------------------------------------------
void GersternWaveSimulation::Simulate()
{
	size_t numSurfacePoints = m_initialSurfacePositions.size();

	for( uint pointIndex = 0; pointIndex < numSurfacePoints; ++pointIndex )
	{
		Vec3 const& initialSurfacePosition = m_initialSurfacePositions[pointIndex];
		Vec3 newSurfacePosition = GetSurfacePositionAtTime( initialSurfacePosition );

		m_surfaceVerts[pointIndex].m_position = newSurfacePosition;
	}

	m_surfaceMesh->UpdateVerticies( static_cast<uint>( m_surfaceVerts.size() ), &m_surfaceVerts[0] );
}


//---------------------------------------------------------------------------------------------------------
Vec3 GersternWaveSimulation::GetSurfacePositionAtTime( Vec3 const& initialPosition )
{
	float totalTimeElapsed = .1f * static_cast<float>(g_theGame->GetGameClock()->GetTotalElapsedSeconds());

	float finalHeight = 0.f;//initialPosition.z;
	Vec2 finalPositionXY = Vec2::ZERO;
	Vec2 initialPositionXY = Vec2( initialPosition.x, initialPosition.y );
	for( int waveIndex = 0; waveIndex < m_waves.size(); ++waveIndex )
	{
		Wave& wave = *m_waves[waveIndex];

		float kDotInitial = DotProduct2D( wave.m_directionNormal, initialPositionXY ) - ( wave.m_frequency * totalTimeElapsed ) + wave.m_phase;

		finalPositionXY += ( ( wave.m_directionNormal / wave.m_magnitude ) * wave.m_amplitude * sinf( kDotInitial ) );
		finalHeight += wave.m_amplitude * cosf( kDotInitial );
	}

	Vec3 finalPosition = initialPosition - Vec3( finalPositionXY, 0.f );
	finalPosition.z = initialPosition.z + finalHeight;
	return finalPosition;
}


