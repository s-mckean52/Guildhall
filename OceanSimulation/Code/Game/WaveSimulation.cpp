#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Game/GameCommon.hpp"
#include "Game/WaveSimulation.hpp"


//---------------------------------------------------------------------------------------------------------
// Wave
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
Wave::Wave( Vec2 const& dir, float wavelength, float setAmplitude, float setPhase )
	: m_amplitude( setAmplitude )
	, m_phase( setPhase )
{
	m_directionNormal = dir;
	if( !ApproximatelyEqual( m_directionNormal.GetLength(), 0.f ) )
	{
		m_directionNormal.Normalize();
	}

	CalculateMagnitudeAndFrequency( wavelength );
}


//---------------------------------------------------------------------------------------------------------
void Wave::CalculateMagnitudeAndFrequency( float waveSize )
{
	m_magnitude = ( 2 * PI_VALUE ) / waveSize;
	m_frequency = sqrtf( 9.81f * m_magnitude );
}


//---------------------------------------------------------------------------------------------------------
void Wave::RotateDirectionDegrees( float degreesToRotate )
{
	m_directionNormal.RotateDegrees( degreesToRotate );
}


//---------------------------------------------------------------------------------------------------------
void Wave::AddAmplitude( float amplitudeToAdd )
{
	m_amplitude += amplitudeToAdd;
	Clamp( m_amplitude, 0.f, 100.f );
}


//---------------------------------------------------------------------------------------------------------
void Wave::AddMagnitude( float magnitudeToAdd )
{
	float waveLength = m_directionNormal.GetLength();
	waveLength += magnitudeToAdd;
	Clamp( waveLength, 0.1f, 100.f );
	m_directionNormal.SetLength( waveLength );
	CalculateMagnitudeAndFrequency( waveLength );
}


//---------------------------------------------------------------------------------------------------------
void Wave::AddPhase( float phaseToAdd )
{
	m_phase += phaseToAdd;
	Clamp( m_phase, 0.f, 100.f );
}


//---------------------------------------------------------------------------------------------------------
// Wave Simulation
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
WaveSimulation::WaveSimulation( Vec2 const& dimensions, uint samples )
	: m_dimensions( dimensions )
	, m_numSamples( samples )
{
	m_transform = new Transform();

	GenerateSurface( Vec3::ZERO, Rgba8::WHITE, dimensions, IntVec2( samples - 1, samples - 1 ) );
	m_surfaceMesh = new GPUMesh( g_theRenderer, m_surfaceVerts, m_surfaceIndicies );
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::Render() const
{
	
	Rgba8 renderColor =  Rgba8::MakeFromFloats( 0.f, 0.412f, 0.58f );
	if( m_isWireFrame ) 
	{ 
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetCullMode( CULL_MODE_BACK );
		g_theRenderer->SetFillMode( FILL_MODE_WIREFRAME );
	}
	else
	{
		g_theRenderer->BindTexture( nullptr );
		g_theRenderer->SetCullMode( CULL_MODE_BACK );
		g_theRenderer->SetFillMode( FILL_MODE_SOLID );
	}

	//g_theRenderer->BindShaderByPath( "Data/Shaders/Normals.hlsl" );
	g_theRenderer->BindMaterialByPath( "Data/Shaders/Lit.material" );

	uint tilingDimSquared = m_tilingDimensions * m_tilingDimensions;
	for( uint i = 0; i < tilingDimSquared; ++i )
	{
		Transform newTransform = *m_transform;
		Vec3 newPosition = Vec3::ZERO;
		newPosition.x = m_dimensions.x * ( i / m_tilingDimensions );
		newPosition.y = m_dimensions.y * ( i % m_tilingDimensions );
		newTransform.Translate( newPosition );

		g_theRenderer->SetModelUBO( newTransform.ToMatrix(), renderColor, 1.0f, 256.f );
		g_theRenderer->DrawMesh( m_surfaceMesh );
	}
}

//---------------------------------------------------------------------------------------------------------
void WaveSimulation::AddWave( Wave* waveToAdd )
{
	m_waves.push_back( waveToAdd );
}


//---------------------------------------------------------------------------------------------------------
int WaveSimulation::GetNumWaves() const
{
	return static_cast<int>( m_waves.size() );
}


//---------------------------------------------------------------------------------------------------------
Wave* WaveSimulation::GetWaveAtIndex( int index ) const
{
	if( m_waves.size() == 0 )
		return nullptr;

	Clamp( index, 0, static_cast<int>( m_waves.size() - 1 ) );
	return m_waves[index];
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::ToggleWireFrameMode()
{
	m_isWireFrame = !m_isWireFrame;
}


//---------------------------------------------------------------------------------------------------------
bool WaveSimulation::IsValidNumSamples( uint numSamples )
{
	if( numSamples < 2 || numSamples > 2048 ) return false;

	if( numSamples & ( numSamples - 1 ) ) return false;

	return true;
}


//---------------------------------------------------------------------------------------------------------
Vec2 WaveSimulation::GetK( int n, int m )
{
	Vec2 k;
	k.x = ( PI_VALUE * ( ( 2.f * static_cast<float>( n ) ) - static_cast<float>( m_numSamples ) ) ) / m_dimensions.x;
	k.y = ( PI_VALUE * ( ( 2.f * static_cast<float>( m ) ) - static_cast<float>( m_numSamples ) ) ) / m_dimensions.y;

	return k;
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::SetPosition( Vec3 const& newPosition )
{
	m_transform->SetPosition( newPosition );
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::SetTilingDimensions( uint tilingDimenisions )
{
	m_tilingDimensions = tilingDimenisions;
}


//---------------------------------------------------------------------------------------------------------
void WaveSimulation::GenerateSurface( Vec3 const& origin, Rgba8 const& color, Vec2 const& dimensions, IntVec2 const& steps )
{
	Vec2 halfDimensions = dimensions * 0.5f;

	float xStepAmount = dimensions.x / static_cast<float>( steps.x );
	float yStepAmount = dimensions.y / static_cast<float>( steps.y );

	float currentY = -halfDimensions.y;
	for( int yStep = 0; yStep < steps.y + 1; ++yStep )
	{
		float currentX = -halfDimensions.x;
		for( int xStep = 0; xStep < steps.x + 1; ++xStep )
		{
			Vec3 currentPosition = origin;
			currentPosition.x += currentX;
			currentPosition.y += currentY;

			float u = RangeMapFloat( -halfDimensions.x, halfDimensions.x, 0.f, 1.f, currentX );
			float v = RangeMapFloat( -halfDimensions.y, halfDimensions.y, 0.f, 1.f, currentY );
			Vec2 uv = Vec2( u, v );

			m_initialSurfacePositions.push_back( currentPosition );
			m_surfaceVerts.push_back( Vertex_PCUTBN( currentPosition, color, Vec3::UNIT_POSITIVE_X, Vec3::UNIT_POSITIVE_Y, Vec3::UNIT_POSITIVE_Z, uv ) );
			currentX += xStepAmount;
		}
		currentY += yStepAmount;
	}

	unsigned int indexOffset = static_cast<unsigned int>( m_surfaceIndicies.size() );
	for( int yIndex = 0; yIndex < steps.y; ++yIndex )
	{
		for( int xIndex = 0; xIndex < steps.x; ++xIndex )
		{
			unsigned int currentVertIndex = xIndex + ( ( steps.x + 1 ) * yIndex );

			unsigned int bottomLeft = currentVertIndex;
			unsigned int bottomRight = currentVertIndex + 1;
			unsigned int topLeft = currentVertIndex + steps.x + 1;
			unsigned int topRight = currentVertIndex + steps.x + 2;

			m_surfaceIndicies.push_back( indexOffset + bottomLeft );
			m_surfaceIndicies.push_back( indexOffset + bottomRight );
			m_surfaceIndicies.push_back( indexOffset + topRight );

			m_surfaceIndicies.push_back( indexOffset + bottomLeft );
			m_surfaceIndicies.push_back( indexOffset + topRight );
			m_surfaceIndicies.push_back( indexOffset + topLeft );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
float WaveSimulation::GetDeepDispersion( Vec2 const& k )
{
	constexpr float w0 = 2.f * 3.14159f / 200.f;
	float wk = sqrtf( GRAVITY * k.GetLength() );
	return RoundDownToInt( wk ) * w0;
}


//---------------------------------------------------------------------------------------------------------
float WaveSimulation::PhillipsEquation( Vec2 const& k )
{
	// A * e^( -1 / ( k * L )^2 ) / k^4 * ( Dot( k, w )^2 )

	float lengthK = k.GetLength();
	if( lengthK <= 0.000001f )
		return 0.f;

	float L = ( m_windSpeed * m_windSpeed ) * INVERSE_GRAVITY;

	float kQuad = lengthK * lengthK * lengthK * lengthK;
	float exponentOfE = -1 / ( ( lengthK * L ) * ( lengthK * L) );
	float eComponent = std::exp( exponentOfE );

	float kDotW = DotProduct2D( k, m_windDirection );
	float kDotWSquared = kDotW * kDotW;

	float damper = 1.f;
	float damperSquared = damper * damper;
	float lengthKSquared = lengthK * lengthK;
	float supressionValue = std::exp( -lengthKSquared * damperSquared );

	return m_A * ( eComponent / kQuad ) * kDotWSquared * supressionValue;
}


//---------------------------------------------------------------------------------------------------------
ComplexFloat WaveSimulation::hTilde( int n, int m, float time )
{
	// 	int halfSamples = static_cast<int>( m_numSamples * 0.5 );
	// 	int index = ( ( m + halfSamples ) * m_numSamples ) + ( n + halfSamples );
	int index = ( m * m_numSamples ) + n;

	Vec2 k = GetK( n, m );

	ComplexFloat htilde0 = m_hTilde0Data[index].m_htilde0;
	ComplexFloat htilde0Conj = m_hTilde0Data[index].m_htilde0Conj;

	float dispersionRelation = GetDeepDispersion( k );
	float dispersionTime = dispersionRelation * time * 10.f;

	float cosDispersionTime = cos( dispersionTime );
	float sinDispersionTime = sin( dispersionTime );

	ComplexFloat eulers( cosDispersionTime, sinDispersionTime );
	ComplexFloat eulersConj( cosDispersionTime, -sinDispersionTime );
	
	return ( htilde0 * eulers ) + ( htilde0Conj * eulersConj );
}


//---------------------------------------------------------------------------------------------------------
ComplexFloat WaveSimulation::hTilde0( int n, int m, bool doesNegateK )
{
	Vec2 k = GetK( n, m );
	if( doesNegateK )
	{
		k = -k;
	}

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