#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Game/FFTWaveSimulation.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include <complex>


//---------------------------------------------------------------------------------------------------------
FFTWaveSimulation::FFTWaveSimulation( Vec2 const& dimensions, uint samples )
	: WaveSimulation( dimensions, samples )
{
}


//---------------------------------------------------------------------------------------------------------
void FFTWaveSimulation::Simulate()
{
	float t = static_cast<float>( g_theGame->GetGameClock()->GetTotalElapsedSeconds() );

	std::vector<Vertex_PCUTBN> wavePoints = m_surfaceVerts;

	for( int wavePointIndex = 0; wavePointIndex < wavePoints.size(); ++wavePointIndex )
	{

		Vec2 position = Vec2::ZERO;
		position.x = m_surfaceVerts[wavePointIndex].m_position.x; //n
		position.y = m_surfaceVerts[wavePointIndex].m_position.y; //m
		
		float n = ( position.x * m_numSamples ) * m_dimensions.x;
		float m = ( position.y * m_numSamples ) * m_dimensions.y;

		Vec2 k = Vec2::ZERO;
		k.x = ( 2 * PI_VALUE * n ) / m_dimensions.x;
		k.y = ( 2 * PI_VALUE * m ) / m_dimensions.y;

		wavePoints[wavePointIndex].m_position.z = CalculateFFTHeight( position, k, t );
	}
	m_surfaceMesh->UpdateVerticies( wavePoints.size(), &wavePoints[0] );
}


//---------------------------------------------------------------------------------------------------------
float FFTWaveSimulation::CalculateFFTHeight( Vec2 const& initialPosition, Vec2 const& k, float timeElapsed )
{
	std::complex<float> kx( 0.f, k.x );
	std::complex<float> ky( 0.f, k.y );
	std::complex<float> totalHeight ( 0, 0 );

	//for( int waveIndex = 0; waveIndex < m_waves.size(); ++waveIndex )
	//{
		const float sqrt_2_under_1 = 1 / sqrtf( 2.f );

		//h0
		float wk = sqrtf( 9.81f * k.GetLength() );
		float er = g_RNG->RollRandomFloatInRange( -100000, 100000 );
		float ei = g_RNG->RollRandomFloatInRange( -100000, 100000 );

		std::complex<float> complexPart( er, ei );
		std::complex<float> complexConjugatePart( er, -ei );

		std::complex<float> h0 = sqrt_2_under_1 * complexPart * PhillipsEquation( k );
		std::complex<float> conjugateH0 = sqrt_2_under_1 * complexConjugatePart * PhillipsEquation( k );
		
		std::complex<float> complexPower = std::exp( std::complex<float>( 0.f, wk * timeElapsed ) );
		std::complex<float> complexPowerConjugate = std::exp( std::complex<float>( 0.f, wk * timeElapsed ) );

		std::complex<float> h = ( h0 * complexPower ) + ( conjugateH0 * complexPowerConjugate );
		totalHeight += h * std::exp( ( kx * initialPosition.x ) + ( ky * initialPosition.y ) );
	//}
	return totalHeight.real();
}

//---------------------------------------------------------------------------------------------------------
float FFTWaveSimulation::PhillipsEquation( Vec2 const& waveDirection )
{
	const float e = 2.71828f;

	// Phillips Spectrum
	float a = 10000000.f;
	Vec2 windDir = Vec2::RIGHT;
	float waveMagnitude = waveDirection.GetLength();
	float waveMagTo4 = waveMagnitude * waveMagnitude * waveMagnitude * waveMagnitude;

	float gravity = 9.81f;
	float windSpeed = 0.1f;
	float l = ( windSpeed * windSpeed ) / gravity;

	float ePow = -1 / ( ( waveMagnitude * l ) * ( waveMagnitude * l ) );
	float windDirDotWaveDir = abs( DotProduct2D( waveDirection.GetNormalized(), windDir.GetNormalized() ) ); 
	float windDirDotWaveDirSquared = windDirDotWaveDir * windDirDotWaveDir;
	float phillipsOfK = a * ( std::exp( ePow ) / waveMagTo4 ) * windDirDotWaveDirSquared;

	return phillipsOfK;
}
