#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Game/DFTWaveSimulation.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
//#include <math.h>


//---------------------------------------------------------------------------------------------------------
DFTWaveSimulation::DFTWaveSimulation( Vec2 const& dimensions, uint samples )
	: WaveSimulation( dimensions, samples )
{
	for( int i = 0; i < m_initialSurfacePositions.size(); ++i )
	{
		Vec3 position = m_initialSurfacePositions[i];
		
		float n = ( position.x * m_numSamples ) / m_dimensions.x;
		float m = ( position.y * m_numSamples ) / m_dimensions.y;

		Vec2 k = Vec2::ZERO;
		k.x = ( 2 * PI_VALUE * n ) / m_dimensions.x;
		k.y = ( 2 * PI_VALUE * m ) / m_dimensions.y;

		HTilde0Data hTildeData;
		hTildeData.m_htilde0		= hTilde0( k );
		hTildeData.m_htilde0Conj	= std::conj( hTilde0( -k ) );
		m_hTildeData.push_back( hTildeData );
	}
}


//---------------------------------------------------------------------------------------------------------
DFTWaveSimulation::~DFTWaveSimulation()
{
}


//---------------------------------------------------------------------------------------------------------
void DFTWaveSimulation::Simulate()
{
	float elapsedTime = static_cast<float>( g_theGame->GetGameClock()->GetTotalElapsedSeconds() );
	for( int positionIndex = 0; positionIndex < m_initialSurfacePositions.size(); ++positionIndex )
	{
		Vec3 initialPosition = m_initialSurfacePositions[positionIndex];
		Vec2 initialPositionXY = Vec2( initialPosition.x, initialPosition.y );
		
		WavePoint wavePoint = GetHeightAtPosition( initialPositionXY, elapsedTime );

		m_surfaceVerts[positionIndex].m_position = initialPosition + Vec3( 0.f, 0.f, wavePoint.m_height.real() );
	}
	m_surfaceMesh->UpdateVerticies( static_cast<uint>( m_surfaceVerts.size() ), &m_surfaceVerts[0] );
}


//---------------------------------------------------------------------------------------------------------
float DFTWaveSimulation::GetDeepDispersion( Vec2 const& k )
{
	constexpr float w0 = 2.f * 3.14159 / 200.f;
	float wk = sqrt( GRAVITY * k.GetLength() );
	return RoundDownToInt( wk ) * w0;
}

//---------------------------------------------------------------------------------------------------------
float DFTWaveSimulation::PhillipsEquation( Vec2 const& k )
{
	// A * e^( -1 / ( k * L )^2 ) / k^4 * ( Dot( k, w )^2 )

	float lengthK = k.GetLength();
	if( lengthK <= 0.000001f )
		return 0.f;

	float L = ( m_windSpeed * m_windSpeed ) * INVERSE_GRAVITY;

	float kQuad = lengthK * lengthK * lengthK * lengthK;
	float exponentOfE = -1 / ( lengthK * L ) * ( lengthK * L);
	float eComponent = std::exp( exponentOfE );

	float kDotW = DotProduct2D( k, m_windDirection );
	float kDotWSquared = kDotW * kDotW;

	float damper = 0.001f;
	float damperSquared = damper * damper;
	float lengthKSquared = lengthK * lengthK;
	float supressionValue = std::exp( -lengthKSquared * damperSquared );

	return m_A * ( eComponent / kQuad ) * kDotWSquared * supressionValue;
}


//---------------------------------------------------------------------------------------------------------
std::complex<float> DFTWaveSimulation::hTilde0( Vec2 const& k )
{
	const float inverse_sqrt_2 = 1.f / sqrtf(2.f);

	//Gaussian;
	float random1 = g_RNG->RollRandomFloatZeroToOneInclusive();
	float random2 = g_RNG->RollRandomFloatZeroToOneInclusive();

	float v = sqrtf( -2 * std::log( random1 ) );
	float f = 2.f * PI_VALUE * random2;

	float gRand1 = v * cos(f);
	float gRand2 = v * sin(f);

	std::complex<float> guassianComplex( gRand1, gRand2 );
	
	return inverse_sqrt_2 * guassianComplex * sqrt( PhillipsEquation( k ) ); 
}


//---------------------------------------------------------------------------------------------------------
std::complex<float> DFTWaveSimulation::hTilde( int index, Vec2 const& k, float time )
{
	std::complex<float> htilde0 = m_hTildeData[index].m_htilde0;
	std::complex<float> htilde0Conj = m_hTildeData[index].m_htilde0Conj;

	float dispersionRelation = GetDeepDispersion( k );
	float dispersionTime = dispersionRelation * time;

	float cosDispersionTime = cos( dispersionTime );
	float sinDispersionTime = sin( dispersionTime );

	std::complex<float> eulers( cosDispersionTime, sinDispersionTime );
	std::complex<float> eulersConj( cosDispersionTime, -sinDispersionTime );
	
	return ( htilde0 * eulers ) + ( htilde0Conj * eulersConj );
}


//---------------------------------------------------------------------------------------------------------
WavePoint DFTWaveSimulation::GetHeightAtPosition( Vec2 const& initialPosition, float time )
{
	std::complex<float> complexHeight;
	Vec2 horizontalDisplacement = Vec2::ZERO;

	for( int initialPositionIndex = 0; initialPositionIndex < m_initialSurfacePositions.size(); ++initialPositionIndex )
	{
		Vec3 position = m_initialSurfacePositions[initialPositionIndex];
		//Vec2 positionXY = Vec2( position.x, position.y );
		
		float n = ( position.x * m_numSamples ) / m_dimensions.x;
		float m = ( position.y * m_numSamples ) / m_dimensions.y;

		Vec2 k = Vec2::ZERO;
		k.x = ( 2 * PI_VALUE * n ) / m_dimensions.x;
		k.y = ( 2 * PI_VALUE * m ) / m_dimensions.y;

		float kDotX = DotProduct2D( initialPosition, k );
		std::complex<float> eulersKDotX( cos( kDotX ), sin( kDotX ) );
		std::complex<float> htilde = hTilde( initialPositionIndex, k, time );

		complexHeight += htilde * eulersKDotX;
		
		if( k.GetLength() < 0.000001f ) continue;
		horizontalDisplacement += ( k / k.GetLength() ) * htilde.imag();
	}

	WavePoint wavePoint;
	wavePoint.m_height = complexHeight;
	wavePoint.m_position = horizontalDisplacement;
	return wavePoint;
}

