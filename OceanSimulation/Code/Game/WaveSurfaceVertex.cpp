#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/WaveSurfaceVertex.hpp"
#include "Game/WaveSimulation.hpp"

//---------------------------------------------------------------------------------------------------------
WaveSurfaceVertex::WaveSurfaceVertex( int xSamplePosition, int ySamplePosition, IntVec2 const& sampleDimensions, Vec2 const& dimensions )
{
	TranslateSurfaceCoord( xSamplePosition, ySamplePosition, sampleDimensions );
	
	float xStep = dimensions.x / static_cast<float>( sampleDimensions.x );
	float yStep = dimensions.y / static_cast<float>( sampleDimensions.y );
	m_initialPosition.x = xSamplePosition * xStep;
	m_initialPosition.y = ySamplePosition * yStep;

	CalculateK( sampleDimensions, dimensions );

	m_hTilde0 = hTilde0();
	m_hTilde0Conj = std::conj( hTilde0( true ) );
}


//---------------------------------------------------------------------------------------------------------
void WaveSurfaceVertex::CalculateHTildeAtTime( float time )
{
	float dispersionRelation = WaveSimulation::GetDeepDispersion( m_k );
	float dispersionTime = dispersionRelation * time * 10.f;

	float cosDispersionTime = cos( dispersionTime );
	float sinDispersionTime = sin( dispersionTime );

	ComplexFloat eulers( cosDispersionTime, sinDispersionTime );
	ComplexFloat eulersConj( cosDispersionTime, -sinDispersionTime );
	
	m_hTilde = ( m_hTilde0 * eulers ) + ( m_hTilde0Conj * eulersConj );
}


//---------------------------------------------------------------------------------------------------------
ComplexFloat WaveSurfaceVertex::CalculateHTilde0( bool doesNegateK )
{
	const float inverse_sqrt_2 = 1.f / sqrtf( 2.f );
	
	Vec2 k = m_k;
	if( doesNegateK )
	{
		k = -m_k;
	}

	//Gaussian;
	float random1 = g_RNG->RollRandomFloatZeroToOneInclusive();
	float random2 = g_RNG->RollRandomFloatZeroToOneInclusive();

	float v = sqrtf( -2 * std::log( random1 ) );
	float f = 2.f * PI_VALUE * random2;

	float gRand1 = v * cos(f);
	float gRand2 = v * sin(f);

	std::complex<float> guassianComplex( gRand1, gRand2 );
	
	return inverse_sqrt_2 * guassianComplex * sqrt( WaveSimulation::PhillipsEquation( k ) ); 
}


//---------------------------------------------------------------------------------------------------------
void WaveSurfaceVertex::CalculateValuesAtTime( float time)
{
	float kLength = m_k.GetLength();

	CalculateHTildeAtTime( time );
	m_surfaceSlope[0] = m_hTilde * ComplexFloat( 0.f, m_k.x );
	m_surfaceSlope[1] = m_hTilde * ComplexFloat( 0.f, m_k.y );
	if( kLength < 0.000001f )
	{
		m_position[0] = ComplexFloat( 0.f, 0.f );
		m_position[1] = ComplexFloat( 0.f, 0.f );
	}
	else
	{
		m_position[0] = m_hTilde * ComplexFloat( 0.f, -m_k.x / kLength );
		m_position[1] = m_hTilde * ComplexFloat( 0.f, -m_k.y / kLength );
	}
}


//---------------------------------------------------------------------------------------------------------
void WaveSurfaceVertex::TranslateSurfaceCoord( int xSamplePosition, int ySamplePosition, IntVec2 const& sampleDimensions )
{
	IntVec2 halfDimensions = sampleDimensions * 0.5f;
	m_translatedCoord = IntVec2( xSamplePosition, ySamplePosition );

	m_translatedCoord -= sampleDimensions;
}


//---------------------------------------------------------------------------------------------------------
void WaveSurfaceVertex::CalculateK( IntVec2 const& sampleDimensions, Vec2 const& dimensions )
{
	m_k.x = ( PI_VALUE * 2.f * static_cast<float>( m_translatedCoord.x ) ) / dimensions.x;
	m_k.y = ( PI_VALUE * 2.f * static_cast<float>( m_translatedCoord.y ) ) / dimensions.y;
}


