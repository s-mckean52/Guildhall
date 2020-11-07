#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Game/WaveSurfaceVertex.hpp"
#include "Game/WaveSimulation.hpp"
#include "Game/FFTWaveSimulation.hpp"

//---------------------------------------------------------------------------------------------------------
WaveSurfaceVertex::WaveSurfaceVertex( WaveSimulation* owner, int xSamplePosition, int ySamplePosition, IntVec2 const& sampleDimensions, Vec2 const& dimensions )
	: m_owner( owner )
{
	TranslateSurfaceCoord( xSamplePosition, ySamplePosition, sampleDimensions );
	
	float xStep = dimensions.x / static_cast<float>( sampleDimensions.x );
	float yStep = dimensions.y / static_cast<float>( sampleDimensions.y );
	m_initialPosition.x = m_translatedCoord.x * xStep;
	m_initialPosition.y = m_translatedCoord.y * yStep;
	m_initialPosition.z = 0.f;

	CalculateK( sampleDimensions, dimensions );

	m_hTilde0 = CalculateHTilde0();
	m_hTilde0Conj = std::conj( CalculateHTilde0( true ) );
}


//---------------------------------------------------------------------------------------------------------
void WaveSurfaceVertex::CalculateHTildeAtTime( float time )
{
	float dispersionRelation = m_owner->GetDeepDispersion( m_k );
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
	
	return inverse_sqrt_2 * guassianComplex * sqrt( m_owner->PhillipsEquation( k ) ); 
}


//---------------------------------------------------------------------------------------------------------
void WaveSurfaceVertex::SetVertexPositionAndNormal( Vertex_PCUTBN& vertexToModify )
{
 	float sign = 1.f - ( 2.f * PositiveMod( m_translatedCoord.x + m_translatedCoord.y, 2 ) );
// 
// 	m_height = m_hTilde.real() * sign;

	float choppiness = m_owner->IsChoppyWater();

	Vec3 translation;
	translation.x = -m_position[0].real() * sign * choppiness;
	translation.y = -m_position[1].real() * sign * choppiness;
	translation.z = m_height;

	m_surfaceNormal.x = m_surfaceSlope[0].real() * sign;
	m_surfaceNormal.y = m_surfaceSlope[1].real() * sign;
	m_surfaceNormal.z = 0.f;

	m_surfaceNormal = ( Vec3::UNIT_POSITIVE_Z - m_surfaceNormal ) / sqrtf( 1.f + DotProduct3D( m_surfaceNormal, m_surfaceNormal ) );
	//normal.Normalize();

	vertexToModify.m_position = m_initialPosition + translation;
	vertexToModify.m_normal = m_surfaceNormal;
	vertexToModify.m_tangent = CrossProduct3D( Vec3::UNIT_POSITIVE_Y, m_surfaceNormal );
	vertexToModify.m_bitangent = CrossProduct3D( m_surfaceNormal, vertexToModify.m_tangent );
}


//---------------------------------------------------------------------------------------------------------
void WaveSurfaceVertex::CalculateValuesAtTime( float time )
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
	IntVec2 halfDimensions;
	halfDimensions.x = static_cast<int>( static_cast<float>( sampleDimensions.x ) * 0.5f );
	halfDimensions.y = static_cast<int>( static_cast<float>( sampleDimensions.y ) * 0.5f );

	m_translatedCoord = IntVec2( xSamplePosition, ySamplePosition ) - halfDimensions;
}


//---------------------------------------------------------------------------------------------------------
void WaveSurfaceVertex::CalculateK( IntVec2 const& sampleDimensions, Vec2 const& dimensions )
{
	UNUSED( sampleDimensions );

	m_k.x = ( PI_VALUE * 2.f * static_cast<float>( m_translatedCoord.x ) ) / dimensions.x;
	m_k.y = ( PI_VALUE * 2.f * static_cast<float>( m_translatedCoord.y ) ) / dimensions.y;
}


