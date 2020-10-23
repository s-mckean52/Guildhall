#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Game/DFTWaveSimulation.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
//#include <math.h>


//---------------------------------------------------------------------------------------------------------
DFTWaveSimulation::DFTWaveSimulation( Vec2 const& dimensions, uint samples, float windSpeed )
	: WaveSimulation( dimensions, samples, windSpeed )
{
	m_hTilde.resize( m_numSamples * m_numSamples );
	for( int i = 0; i < m_initialSurfacePositions.size(); ++i )
	{
		int m = i / m_numSamples;
		int n = i - ( m * m_numSamples );

		HTilde0Data hTildeData;
		hTildeData.m_htilde0		= hTilde0( n, m );
		hTildeData.m_htilde0Conj	= std::conj( hTilde0( n, m, true ) );
		m_hTilde0Data.push_back( hTildeData );
	}
}


//---------------------------------------------------------------------------------------------------------
DFTWaveSimulation::~DFTWaveSimulation()
{
}


//---------------------------------------------------------------------------------------------------------
void DFTWaveSimulation::Simulate()
{
	float elapsedTime = static_cast<float>(g_theGame->GetGameClock()->GetTotalElapsedSeconds());
	for( int positionIndex = 0; positionIndex < m_initialSurfacePositions.size(); ++positionIndex )
	{
		Vec3 initialPosition = m_initialSurfacePositions[positionIndex];
		Vec2 initialPositionXY = Vec2( initialPosition.x, initialPosition.y );
		
		WavePoint wavePoint = GetHeightAtPosition( initialPositionXY, elapsedTime );

		Vec3 translation;
		translation.x = 0.f;	//wavePoint.m_position.x;
		translation.y = 0.f;	//wavePoint.m_position.y;
		translation.z = wavePoint.m_height.real();

		m_hTilde[positionIndex] = wavePoint.m_height;
		m_surfaceVerts[positionIndex].m_position = initialPosition + translation;
	}
	m_surfaceMesh->UpdateVerticies( static_cast<uint>( m_surfaceVerts.size() ), &m_surfaceVerts[0] );
}


//---------------------------------------------------------------------------------------------------------
WavePoint DFTWaveSimulation::GetHeightAtPosition( Vec2 const& initialPosition, float time )
{
	std::complex<float> complexHeight;
	Vec2 horizontalDisplacement = Vec2::ZERO;

	for( int positionIndex = 0; positionIndex < m_initialSurfacePositions.size(); ++positionIndex )
	{
		int m = positionIndex / m_numSamples;			// 0 < n < numSamples  
		int n = positionIndex - ( m * m_numSamples );	// 0 < m < numSamples

		Vec2 k = GetK( n, m );

		float kDotX = DotProduct2D( initialPosition, k );
		std::complex<float> eulersKDotX( cos( kDotX ), sin( kDotX ) );
		std::complex<float> htilde = hTilde( n, m, time );

		complexHeight += htilde * eulersKDotX;
		
		if( k.GetLength() < 0.000001f ) continue;
		horizontalDisplacement += ( k / k.GetLength() ) * htilde.imag();
	}

	WavePoint wavePoint;
	wavePoint.m_height = complexHeight;
	wavePoint.m_position = horizontalDisplacement;
	return wavePoint;
}

