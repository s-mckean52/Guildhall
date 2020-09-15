#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Game/GerstnerWaveSimulation.hpp"
#include "Game/Game.hpp"

//---------------------------------------------------------------------------------------------------------
GersternWaveSimulation::GersternWaveSimulation( Vec2 const& dimensions, uint samples )
	: WaveSimulation( dimensions, samples )
{
}


//---------------------------------------------------------------------------------------------------------
void GersternWaveSimulation::Simulate()
{
	size_t numSurfacePoints = m_surfaceVerts.size();

	std::vector<Vertex_PCUTBN> gerstnerWaveVerts = m_surfaceVerts;

	for( uint pointIndex = 0; pointIndex < numSurfacePoints; ++pointIndex )
	{
		Vec3 initialPosition = m_surfaceVerts[pointIndex].m_position;
		initialPosition.z = 0.f;

		Vec3 finalPosition = GetWaveVectorSums( initialPosition );

		gerstnerWaveVerts[pointIndex].m_position = finalPosition;
	}

	m_surfaceMesh->UpdateVerticies( static_cast<uint>( gerstnerWaveVerts.size() ), &gerstnerWaveVerts[0] );
}


//---------------------------------------------------------------------------------------------------------
Vec3 GersternWaveSimulation::GetWaveVectorSums( Vec3 const& initialPosition )
{
	Vec3 finalPosition = Vec3::ZERO;
	float finalHeight = 0.f;

	float t = static_cast<float>(g_theGame->GetGameClock()->GetTotalElapsedSeconds());
	for( int waveIndex = 0; waveIndex < m_waves.size(); ++waveIndex )
	{
		Wave* wave = m_waves[waveIndex];
		Vec3 waveDir3D = Vec3( wave->direction, 0.f );

		float kDotInitial = DotProduct3D( waveDir3D.GetNormalize(), initialPosition ) - ( wave->frequency * t ) + wave->phase;

		finalPosition += ( ( waveDir3D / wave->magnitude ) * wave->amplitude * sinf( kDotInitial ) );
		finalHeight += wave->amplitude * cosf( kDotInitial );
	}

	finalPosition = initialPosition - finalPosition;
	finalPosition.z = finalHeight;

	return finalPosition;
}


